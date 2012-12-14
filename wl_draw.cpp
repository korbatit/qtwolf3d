#include "wl_def.h"

#include "wlpage.h"
#include "wlinput.h"
#include "wlcache.h"
#include "wlitem.h"
#include "wlpaint.h"
#include "wluser.h"

extern WLPage*  wlpage;
extern WLCache* wlcache;
extern WLInput* wlinput;
extern WLItem*  wlitem;
extern WLPaint* wlpaint;
extern WLUser*  wluser;

extern QImage* g_Image;
extern QTime* g_getTicks;

extern quint16  PrintX;
extern quint16  PrintY;

extern void gsleep(int ms);

#define ACTORSIZE       0x4000

static quint8 *vbuf = NULL;
unsigned vbufPitch = 0;

qint32    lasttimecount;
qint32    frameon;
qint8 fpscounter;

int fps_frames=0, fps_time=0, fps=0;

int *wallheight;
int min_wallheight;

short *pixelangle;
qint32 finetangent[FINEANGLES/4];
qint32 sintable[ANGLES+ANGLES/4];
qint32 *costable = sintable+(ANGLES/4);

qint32   viewx,viewy;                    // the focal point
short   viewangle;
qint32   viewsin,viewcos;

void    TransformActor (objtype *ob);
void    BuildTables (void);
void    ClearScreen (void);
int     CalcRotate (objtype *ob);
void    DrawScaleds (void);
void    CalcTics (void);
void    ThreeDRefresh (void);


int     lastside;               // true for vertical
qint32    lastintercept;
int     lasttilehit;
int     lasttexture;

short    focaltx,focalty,viewtx,viewty;
quint32 xpartialup,xpartialdown,ypartialup,ypartialdown;

short   midangle,angle;

quint16    tilehit;
int     pixx;

short   xtile,ytile;
short   xtilestep,ytilestep;
qint32    xintercept,yintercept;
quint16    xstep,ystep;
quint16    xspot,yspot;
int     texdelta;

quint16 horizwall[MAXWALLTILES],vertwall[MAXWALLTILES];

void TransformActor (objtype *ob)
{
    qint32 gx,gy,gxt,gyt,nx,ny;

    gx = ob->x-viewx;
    gy = ob->y-viewy;
    gxt = FixedMul(gx,viewcos);
    gyt = FixedMul(gy,viewsin);
    nx = gxt-gyt-ACTORSIZE;         // fudge the shape forward a bit, because
    gxt = FixedMul(gx,viewsin);
    gyt = FixedMul(gy,viewcos);
    ny = gyt+gxt;
    ob->transx = nx;
    ob->transy = ny;

    if (nx<MINDIST) {
        ob->viewheight = 0;
        return;
    }
    ob->viewx = (quint16)(centerx + ny*scale/nx);
    ob->viewheight = (quint16)(heightnumerator/(nx>>8));
}

qint8 TransformTile (int tx, int ty, short *dispx, short *dispheight)
{
    qint32 gx,gy,gxt,gyt,nx,ny;

    gx = ((qint32)tx<<TILESHIFT)+0x8000-viewx;
    gy = ((qint32)ty<<TILESHIFT)+0x8000-viewy;
    gxt = FixedMul(gx,viewcos);
    gyt = FixedMul(gy,viewsin);
    nx = gxt-gyt-0x2000;            // 0x2000 is size of object
    gxt = FixedMul(gx,viewsin);
    gyt = FixedMul(gy,viewcos);
    ny = gyt+gxt;
    if (nx<MINDIST)                 // too close, don't overflow the divide
        *dispheight = 0;
    else {
        *dispx = (short)(centerx + ny*scale/nx);
        *dispheight = (short)(heightnumerator/(nx>>8));
    }
    if (nx<TILEGLOBAL && ny>-TILEGLOBAL/2 && ny<TILEGLOBAL/2)
        return true;
    else
        return false;
}

int CalcHeight()
{
    qint32 z = FixedMul(xintercept - viewx, viewcos)
            - FixedMul(yintercept - viewy, viewsin);
    if(z < MINDIST) z = MINDIST;
    int height = heightnumerator / (z >> 8);
    if(height < min_wallheight) min_wallheight = height;
    return height;
}

quint8 *postsource;
int postx;
int postwidth;

void ScalePost()
{
    int ywcount, yoffs, yw, yd, yendoffs;
    quint8 col;

    ywcount = yd = wallheight[postx] >> 3;
    if(yd <= 0) yd = 100;

    yoffs = (viewheight / 2 - ywcount) * vbufPitch;
    if(yoffs < 0) yoffs = 0;
    yoffs += postx;

    yendoffs = viewheight / 2 + ywcount - 1;
    yw=TEXTURESIZE-1;

    while(yendoffs >= viewheight) {
        ywcount -= TEXTURESIZE/2;
        while(ywcount <= 0) {
            ywcount += yd;
            yw--;
        }
        yendoffs--;
    }
    if(yw < 0) return;

    col = postsource[yw];
    yendoffs = yendoffs * vbufPitch + postx;
    while(yoffs <= yendoffs) {
        vbuf[yendoffs] = col;
        ywcount -= TEXTURESIZE/2;
        if(ywcount <= 0) {
            do {
                ywcount += yd;
                yw--;
            }
            while(ywcount <= 0);
            if(yw < 0) break;
            col = postsource[yw];
        }
        yendoffs -= vbufPitch;
    }
}

void GlobalScalePost(quint8 *vidbuf, unsigned pitch)
{
    vbuf = vidbuf;
    vbufPitch = pitch;
    ScalePost();
}

void HitVertWall (void)
{
    int wallpic;
    int texture;

    texture = ((yintercept+texdelta)>>TEXTUREFROMFIXEDSHIFT)&TEXTUREMASK;
    if (xtilestep == -1) {
        texture = TEXTUREMASK-texture;
        xintercept += TILEGLOBAL;
    }

    if(lastside==1 && lastintercept==xtile && lasttilehit==tilehit && !(lasttilehit & 0x40)) {
        if((pixx&3) && texture == lasttexture) {
            ScalePost();
            postx = pixx;
            wallheight[pixx] = wallheight[pixx-1];
            return;
        }
        ScalePost();
        wallheight[pixx] = CalcHeight();
        postsource+=texture-lasttexture;
        postwidth=1;
        postx=pixx;
        lasttexture=texture;
        return;
    }

    if(lastside!=-1) ScalePost();

    lastside=1;
    lastintercept=xtile;
    lasttilehit=tilehit;
    lasttexture=texture;
    wallheight[pixx] = CalcHeight();
    postx = pixx;
    postwidth = 1;

    if (tilehit & 0x40) {  // check for adjacent doors
        ytile = (short)(yintercept>>TILESHIFT);
        if ( tilemap[xtile-xtilestep][ytile]&0x80 )
            wallpic = (wlpage->spriteStart()-8)+3;
        else
            wallpic = vertwall[tilehit & ~0x40];
    } else
        wallpic = vertwall[tilehit];
    postsource = (quint8*)wlpage->texture(wallpic) + texture;
}

void HitHorizWall (void)
{
    int wallpic;
    int texture;

    texture = ((xintercept+texdelta)>>TEXTUREFROMFIXEDSHIFT)&TEXTUREMASK;
    if (ytilestep == -1)
        yintercept += TILEGLOBAL;
    else
        texture = TEXTUREMASK-texture;

    if(lastside==0 && lastintercept==ytile && lasttilehit==tilehit && !(lasttilehit & 0x40)) {
        if((pixx&3) && texture == lasttexture) {
            ScalePost();
            postx=pixx;
            wallheight[pixx] = wallheight[pixx-1];
            return;
        }
        ScalePost();
        wallheight[pixx] = CalcHeight();
        postsource+=texture-lasttexture;
        postwidth=1;
        postx=pixx;
        lasttexture=texture;
        return;
    }

    if(lastside!=-1) ScalePost();

    lastside=0;
    lastintercept=ytile;
    lasttilehit=tilehit;
    lasttexture=texture;
    wallheight[pixx] = CalcHeight();
    postx = pixx;
    postwidth = 1;

    if (tilehit & 0x40) { // check for adjacent doors
        xtile = (short)(xintercept>>TILESHIFT);
        if ( tilemap[xtile][ytile-ytilestep]&0x80)
            wallpic = (wlpage->spriteStart()-8)+2;
        else
            wallpic = horizwall[tilehit & ~0x40];
    } else
        wallpic = horizwall[tilehit];
    postsource = (quint8*)wlpage->texture(wallpic) + texture;
}

void HitHorizDoor (void)
{
    int doorpage = 0;
    int doornum;
    int texture;

    doornum = tilehit&0x7f;
    texture = ((xintercept - *wlitem->doorPosition(doornum)) >> TEXTUREFROMFIXEDSHIFT)&TEXTUREMASK;

    if(lasttilehit==tilehit) {
        if((pixx&3) && texture == lasttexture) {
            ScalePost();
            postx=pixx;
            wallheight[pixx] = wallheight[pixx-1];
            return;
        }
        ScalePost();
        wallheight[pixx] = CalcHeight();
        postsource+=texture-lasttexture;
        postwidth=1;
        postx=pixx;
        lasttexture=texture;
        return;
    }

    if(lastside!=-1) ScalePost();

    lastside=2;
    lasttilehit=tilehit;
    lasttexture=texture;
    wallheight[pixx] = CalcHeight();
    postx = pixx;
    postwidth = 1;

    switch(wlitem->doorObjList(doornum)->lock) {
    case dr_normal:
        doorpage = wlpage->spriteStart()-8;
        break;
    case dr_lock1:
    case dr_lock2:
    case dr_lock3:
    case dr_lock4:
        doorpage = wlpage->spriteStart()-8+6;
        break;
    case dr_elevator:
        doorpage = wlpage->spriteStart()+4;
        break;
    }
    postsource = (quint8*)wlpage->texture(doorpage) + texture;
}

void HitVertDoor (void)
{
    int doorpage = 0;
    int doornum;
    int texture;

    doornum = tilehit&0x7f;
    texture = ((yintercept - *wlitem->doorPosition(doornum)) >> TEXTUREFROMFIXEDSHIFT)&TEXTUREMASK;

    if(lasttilehit==tilehit) {
        if((pixx&3) && texture == lasttexture) {
            ScalePost();
            postx=pixx;
            wallheight[pixx] = wallheight[pixx-1];
            return;
        }
        ScalePost();
        wallheight[pixx] = CalcHeight();
        postsource+=texture-lasttexture;
        postwidth=1;
        postx=pixx;
        lasttexture=texture;
        return;
    }

    if(lastside!=-1) ScalePost();

    lastside=2;
    lasttilehit=tilehit;
    lasttexture=texture;
    wallheight[pixx] = CalcHeight();
    postx = pixx;
    postwidth = 1;

    switch(wlitem->doorObjList(doornum)->lock) {
    case dr_normal:
        doorpage = wlpage->spriteStart()-8+1;
        break;
    case dr_lock1:
    case dr_lock2:
    case dr_lock3:
    case dr_lock4:
        doorpage = wlpage->spriteStart()-8+7;
        break;
    case dr_elevator:
        doorpage = wlpage->spriteStart()-8+5;
        break;
    }
    postsource = (quint8*)wlpage->texture(doorpage) + texture;
}

void HitHorizPWall (void)
{
    int wallpic;
    int texture,offset;

    texture = (xintercept>>TEXTUREFROMFIXEDSHIFT)&TEXTUREMASK;
    offset = *wlitem->pWallPos()<<10;
    if (ytilestep == -1)
        yintercept += TILEGLOBAL-offset;
    else {
        texture = TEXTUREMASK-texture;
        yintercept += offset;
    }

    if(lasttilehit==tilehit && lastside==0) {
        if((pixx&3) && texture == lasttexture)
        {
            ScalePost();
            postx=pixx;
            wallheight[pixx] = wallheight[pixx-1];
            return;
        }
        ScalePost();
        wallheight[pixx] = CalcHeight();
        postsource+=texture-lasttexture;
        postwidth=1;
        postx=pixx;
        lasttexture=texture;
        return;
    }

    if(lastside!=-1) ScalePost();

    lastside=0;
    lasttilehit=tilehit;
    lasttexture=texture;
    wallheight[pixx] = CalcHeight();
    postx = pixx;
    postwidth = 1;

    wallpic = horizwall[*wlitem->pWallTile()&63];
    postsource = (quint8*)wlpage->texture(wallpic) + texture;
}

void HitVertPWall (void)
{
    int wallpic;
    int texture,offset;

    texture = (yintercept>>TEXTUREFROMFIXEDSHIFT)&TEXTUREMASK;
    offset = *wlitem->pWallPos()<<10;
    if (xtilestep == -1) {
        xintercept += TILEGLOBAL-offset;
        texture = TEXTUREMASK-texture;
    } else
        xintercept += offset;

    if(lasttilehit==tilehit && lastside==1) {
        if((pixx&3) && texture == lasttexture) {
            ScalePost();
            postx=pixx;
            wallheight[pixx] = wallheight[pixx-1];
            return;
        }
        ScalePost();
        wallheight[pixx] = CalcHeight();
        postsource+=texture-lasttexture;
        postwidth=1;
        postx=pixx;
        lasttexture=texture;
        return;
    }

    if(lastside!=-1) ScalePost();

    lastside=1;
    lasttilehit=tilehit;
    lasttexture=texture;
    wallheight[pixx] = CalcHeight();
    postx = pixx;
    postwidth = 1;

    wallpic = vertwall[*wlitem->pWallTile()&63];
    postsource = (quint8*)wlpage->texture(wallpic) + texture;
}

#define HitHorizBorder HitHorizWall
#define HitVertBorder HitVertWall

quint8 vgaCeiling[]=
{
    #ifndef SPEAR
    0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0xbf,
    0x4e,0x4e,0x4e,0x1d,0x8d,0x4e,0x1d,0x2d,0x1d,0x8d,
    0x1d,0x1d,0x1d,0x1d,0x1d,0x2d,0xdd,0x1d,0x1d,0x98,

    0x1d,0x9d,0x2d,0xdd,0xdd,0x9d,0x2d,0x4d,0x1d,0xdd,
    0x7d,0x1d,0x2d,0x2d,0xdd,0xd7,0x1d,0x1d,0x1d,0x2d,
    0x1d,0x1d,0x1d,0x1d,0xdd,0xdd,0x7d,0xdd,0xdd,0xdd
    #else
    0x6f,0x4f,0x1d,0xde,0xdf,0x2e,0x7f,0x9e,0xae,0x7f,
    0x1d,0xde,0xdf,0xde,0xdf,0xde,0xe1,0xdc,0x2e,0x1d,0xdc
    #endif
};

void VGAClearScreen (void)
{
    quint8 ceiling=vgaCeiling[gamestate.episode*10+wlcache->currentMap()];

    int y;
    quint8 *ptr = vbuf;
    for(y = 0; y < viewheight / 2; y++, ptr += vbufPitch)
        memset(ptr, ceiling, viewwidth);
    for(; y < viewheight; y++, ptr += vbufPitch)
        memset(ptr, 0x19, viewwidth);
}

int CalcRotate (objtype *ob)
{
    int angle, viewangle;

    viewangle = player->angle + (centerx - ob->viewx)/8;

    if (ob->obclass == rocketobj || ob->obclass == hrocketobj)
        angle = (viewangle-180) - ob->angle;
    else
        angle = (viewangle-180) - dirangle[ob->dir];

    angle+=ANGLES/16;
    while (angle>=ANGLES)
        angle-=ANGLES;
    while (angle<0)
        angle+=ANGLES;

    if (ob->state->rotate == 2)             // 2 rotation pain frame
        return 0;               // pain with shooting frame bugfix

    return angle/(ANGLES/8);
}

void ScaleShape (int xcenter, int shapenum, unsigned height, quint32 flags)
{
    Q_UNUSED(flags)

    t_compshape *shape;
    unsigned scale,pixheight;
    unsigned starty,endy;
    quint16 *cmdptr;
    quint8 *cline;
    quint8 *line;
    quint8 *vmem;
    int actx,i,upperedge;
    short newstart;
    int scrstarty,screndy,lpix,rpix,pixcnt,ycnt;
    unsigned j;
    quint8 col;

    shape = (t_compshape*)wlpage->sprite(shapenum);

    scale=height>>3;                 // low three bits are fractional
    if(!scale) return;   // too close or far away

    pixheight=scale*SPRITESCALEFACTOR;
    actx=xcenter-scale;
    upperedge=viewheight/2-scale;

    cmdptr=(quint16 *) shape->dataofs;

    for(i=shape->leftpix,pixcnt=i*pixheight,rpix=(pixcnt>>6)+actx;i<=shape->rightpix;i++,cmdptr++) {
        lpix=rpix;
        if(lpix>=viewwidth) break;
        pixcnt+=pixheight;
        rpix=(pixcnt>>6)+actx;
        if(lpix!=rpix && rpix>0) {
            if(lpix<0) lpix=0;
            if(rpix>viewwidth) rpix=viewwidth,i=shape->rightpix+1;
            cline=(quint8*)shape + *cmdptr;
            while(lpix<rpix) {
                if(wallheight[lpix]<=(int)height) {
                    line=cline;
                    while((endy = READWORD(line)) != 0) {
                        endy >>= 1;
                        newstart = READWORD(line);
                        starty = READWORD(line) >> 1;
                        j=starty;
                        ycnt=j*pixheight;
                        screndy=(ycnt>>6)+upperedge;
                        if(screndy<0) vmem=vbuf+lpix;
                        else vmem=vbuf+screndy*vbufPitch+lpix;
                        for(;j<endy;j++) {
                            scrstarty=screndy;
                            ycnt+=pixheight;
                            screndy=(ycnt>>6)+upperedge;
                            if(scrstarty!=screndy && screndy>0) {
                                col=((quint8*)shape)[newstart+j];
                                if(scrstarty<0) scrstarty=0;
                                if(screndy>viewheight) screndy=viewheight,j=endy;

                                while(scrstarty<screndy) {
                                    *vmem=col;
                                    vmem+=vbufPitch;
                                    scrstarty++;
                                }
                            }
                        }
                    }
                }
                lpix++;
            }
        }
    }
}

void SimpleScaleShape (int xcenter, int shapenum, unsigned height)
{
    t_compshape   *shape;
    unsigned scale,pixheight;
    unsigned starty,endy;
    quint16 *cmdptr;
    quint8 *cline;
    quint8 *line;
    int actx,i,upperedge;
    short newstart;
    int scrstarty,screndy,lpix,rpix,pixcnt,ycnt;
    unsigned j;
    quint8 col;
    quint8 *vmem;

    shape = (t_compshape*)wlpage->sprite(shapenum);
    scale=height>>1;
    pixheight=scale*SPRITESCALEFACTOR;
    actx=xcenter-scale;
    upperedge=viewheight/2-scale;
    cmdptr=shape->dataofs;
    for(i=shape->leftpix,pixcnt=i*pixheight,rpix=(pixcnt>>6)+actx;i<=shape->rightpix;i++,cmdptr++) {
        lpix=rpix;
        if(lpix>=viewwidth) break;
        pixcnt+=pixheight;
        rpix=(pixcnt>>6)+actx;
        if(lpix!=rpix && rpix>0) {
            if(lpix<0) lpix=0;
            if(rpix>viewwidth) rpix=viewwidth,i=shape->rightpix+1;
            cline = (quint8*)shape + *cmdptr;
            while(lpix<rpix) {
                line=cline;
                while((endy = READWORD(line)) != 0) {
                    endy >>= 1;
                    newstart = READWORD(line);
                    starty = READWORD(line) >> 1;
                    j=starty;
                    ycnt=j*pixheight;
                    screndy=(ycnt>>6)+upperedge;
                    if(screndy<0) vmem=vbuf+lpix;
                    else vmem=vbuf+screndy*vbufPitch+lpix;
                    for(;j<endy;j++) {
                        scrstarty=screndy;
                        ycnt+=pixheight;
                        screndy=(ycnt>>6)+upperedge;
                        if(scrstarty!=screndy && screndy>0) {
                            col=((quint8*)shape)[newstart+j];
                            if(scrstarty<0) scrstarty=0;
                            if(screndy>viewheight) screndy=viewheight,j=endy;
                            while(scrstarty<screndy) {
                                *vmem=col;
                                vmem+=vbufPitch;
                                scrstarty++;
                            }
                        }
                    }
                }
                lpix++;
            }
        }
    }
}

#define MAXVISABLE 250

typedef struct {
    short      viewx,
    viewheight,
    shapenum;
    short      flags;          // this must be changed to quint32, when you
    // you need more than 16-flags for drawing
} visobj_t;

visobj_t vislist[MAXVISABLE];
visobj_t *visptr,*visstep,*farthest;

void DrawScaleds (void)
{
    int      i,least,numvisable,height;
    quint8     *tilespot,*visspot;
    unsigned spotloc;

    statObj_t *statptr;
    objtype   *obj;

    visptr = &vislist[0];

    for (statptr = &statobjlist[0] ; statptr !=laststatobj ; statptr++) {
        if ((visptr->shapenum = statptr->shapeNum) == -1)
            continue;                                               // object has been deleted

        if (!*statptr->visSpot)
            continue;                                               // not visable

        if (TransformTile (statptr->tileX,statptr->tileY,
                           &visptr->viewx,&visptr->viewheight) && statptr->flags & FL_BONUS) {
            GetBonus (statptr);
            if(statptr->shapeNum == -1)
                continue;                                           // object has been taken
        }

        if (!visptr->viewheight)
            continue;                                               // to close to the object

        if (visptr < &vislist[MAXVISABLE-1]) {
            visptr->flags = (short) statptr->flags;
            visptr++;
        }
    }
    for (obj = player->next;obj;obj=obj->next) {
        if ((visptr->shapenum = obj->state->shapenum)==0)
            continue;                                               // no shape

        spotloc = (obj->tilex<<mapshift)+obj->tiley;   // optimize: keep in struct?
        visspot = &spotvis[0][0]+spotloc;
        tilespot = &tilemap[0][0]+spotloc;

        if (*visspot
                || ( *(visspot-1) && !*(tilespot-1) )
                || ( *(visspot+1) && !*(tilespot+1) )
                || ( *(visspot-65) && !*(tilespot-65) )
                || ( *(visspot-64) && !*(tilespot-64) )
                || ( *(visspot-63) && !*(tilespot-63) )
                || ( *(visspot+65) && !*(tilespot+65) )
                || ( *(visspot+64) && !*(tilespot+64) )
                || ( *(visspot+63) && !*(tilespot+63) ) ) {
            obj->active = ac_yes;
            TransformActor (obj);
            if (!obj->viewheight)
                continue;                                               // too close or far away

            visptr->viewx = obj->viewx;
            visptr->viewheight = obj->viewheight;
            if (visptr->shapenum == -1)
                visptr->shapenum = obj->temp1;  // special shape

            if (obj->state->rotate)
                visptr->shapenum += CalcRotate (obj);

            if (visptr < &vislist[MAXVISABLE-1]) {
                visptr->flags = (short) obj->flags;
                visptr++;
            }
            obj->flags |= FL_VISABLE;
        }
        else
            obj->flags &= ~FL_VISABLE;
    }

    numvisable = (int) (visptr-&vislist[0]);

    if (!numvisable)
        return;                                                                 // no visable objects

    for (i = 0; i<numvisable; i++) {
        least = 32000;
        for (visstep=&vislist[0] ; visstep<visptr ; visstep++) {
            height = visstep->viewheight;
            if (height < least)
            {
                least = height;
                farthest = visstep;
            }
        }
        ScaleShape(farthest->viewx, farthest->shapenum, farthest->viewheight, farthest->flags);
        farthest->viewheight = 32000;
    }
}

int weaponscale[NUMWEAPONS] = {SPR_KNIFEREADY, SPR_PISTOLREADY,
                               SPR_MACHINEGUNREADY, SPR_CHAINREADY};

void DrawPlayerWeapon (void)
{
    int shapenum;

    if (gamestate.victoryflag) {
        if (player->state == &s_deathcam && ((g_getTicks->elapsed()*7/100)&32) )
            SimpleScaleShape(viewwidth/2,SPR_DEATHCAM,viewheight+1);
        return;
    }
    if (gamestate.weapon != -1) {
        shapenum = weaponscale[gamestate.weapon]+gamestate.weaponframe;
        SimpleScaleShape(viewwidth/2,shapenum,viewheight+1);
    }
    if (demorecord || demoplayback)
        SimpleScaleShape(viewwidth/2,SPR_DEMO,viewheight+1);
}

void CalcTics (void)
{
    if (lasttimecount > (qint32) g_getTicks->elapsed()*7/100)
        lasttimecount = g_getTicks->elapsed()*7/100;    // if the game was paused a LONG time

    quint32 curtime = g_getTicks->elapsed();
    tics = (curtime * 7) / 100 - lasttimecount;
    if(!tics) {
        // wait until end of current tic
        gsleep(((lasttimecount + 1) * 100) / 7 - curtime);
        tics = 1;
    }
    lasttimecount += tics;
    if (tics>MAXTICS)
        tics = MAXTICS;
}

void AsmRefresh()
{
    qint32 xstep = 0, ystep = 0;
    quint32 xpartial = 0, ypartial = 0;
    qint8 playerInPushwallBackTile = tilemap[focaltx][focalty] == 64;

    for(pixx=0;pixx<viewwidth;pixx++) {
        short angl=midangle+pixelangle[pixx];
        if(angl<0) angl+=FINEANGLES;
        if(angl>=3600) angl-=FINEANGLES;
        if(angl<900) {
            xtilestep=1;
            ytilestep=-1;
            xstep=finetangent[900-1-angl];
            ystep=-finetangent[angl];
            xpartial=xpartialup;
            ypartial=ypartialdown;
        } else if(angl<1800) {
            xtilestep=-1;
            ytilestep=-1;
            xstep=-finetangent[angl-900];
            ystep=-finetangent[1800-1-angl];
            xpartial=xpartialdown;
            ypartial=ypartialdown;
        } else if(angl<2700) {
            xtilestep=-1;
            ytilestep=1;
            xstep=-finetangent[2700-1-angl];
            ystep=finetangent[angl-1800];
            xpartial=xpartialdown;
            ypartial=ypartialup;
        } else if(angl<3600) {
            xtilestep=1;
            ytilestep=1;
            xstep=finetangent[angl-2700];
            ystep=finetangent[3600-1-angl];
            xpartial=xpartialup;
            ypartial=ypartialup;
        }
        yintercept=FixedMul(ystep,xpartial)+viewy;
        xtile=focaltx+xtilestep;
        xspot=(quint16)((xtile<<mapshift)+((quint32)yintercept>>16));
        xintercept=FixedMul(xstep,ypartial)+viewx;
        ytile=focalty+ytilestep;
        yspot=(quint16)((((quint32)xintercept>>16)<<mapshift)+ytile);
        texdelta=0;

        // Special treatment when player is in back tile of pushwall
        if(playerInPushwallBackTile) {
            if(    (*wlitem->pWallDir() == di_east && xtilestep ==  1)
                    || (*wlitem->pWallDir() == di_west && xtilestep == -1)) {
                qint32 yintbuf = yintercept - ((ystep * (64 - *wlitem->pWallPos())) >> 6);
                if((yintbuf >> 16) == focalty) {
                    if(*wlitem->pWallDir() == di_east)
                        xintercept = (focaltx << TILESHIFT) + (*wlitem->pWallPos() << 10);
                    else
                        xintercept = (focaltx << TILESHIFT) - TILEGLOBAL + ((64 - *wlitem->pWallPos()) << 10);
                    yintercept = yintbuf;
                    tilehit = *wlitem->pWallTile();
                    HitVertWall();
                    continue;
                }
            } else if((*wlitem->pWallDir() == di_south && ytilestep ==  1)
                      ||  (*wlitem->pWallDir() == di_north && ytilestep == -1)) {
                qint32 xintbuf = xintercept - ((xstep * (64 - *wlitem->pWallPos())) >> 6);
                if((xintbuf >> 16) == focaltx) {
                    xintercept = xintbuf;
                    if(*wlitem->pWallDir() == di_south)
                        yintercept = (focalty << TILESHIFT) + (*wlitem->pWallPos() << 10);
                    else
                        yintercept = (focalty << TILESHIFT) - TILEGLOBAL + ((64 - *wlitem->pWallPos()) << 10);
                    tilehit = *wlitem->pWallTile();
                    HitHorizWall();
                    continue;
                }
            }
        }
        do {
            if(ytilestep==-1 && (yintercept>>16)<=ytile) goto horizentry;
            if(ytilestep==1 && (yintercept>>16)>=ytile) goto horizentry;
            vertentry:
                if((quint32)yintercept>mapheight*65536-1 || (quint16)xtile>=mapwidth) {
                if(xtile<0) xintercept=0;
                if(xtile>=mapwidth) xintercept=mapwidth<<TILESHIFT;
                if(yintercept<0) yintercept=0;
                if(yintercept>=(mapheight<<TILESHIFT)) yintercept=mapheight<<TILESHIFT;
                yspot=0xffff;
                tilehit=0;
                HitHorizBorder();
                break;
            }
            if(xspot>=maparea) break;
            tilehit=((quint8*)tilemap)[xspot];
            if(tilehit) {
                if(tilehit&0x80) {
                    qint32 yintbuf=yintercept+(ystep>>1);
                    if((yintbuf>>16)!=(yintercept>>16))
                        goto passvert;
                    if((quint16)yintbuf < *wlitem->doorPosition(tilehit&0x7f))
                        goto passvert;
                    yintercept=yintbuf;
                    xintercept=(xtile<<TILESHIFT)|0x8000;
                    HitVertDoor();
                } else {
                    if(tilehit==64) {
                        if(*wlitem->pWallDir() == di_west || *wlitem->pWallDir() == di_east) {
                            qint32 yintbuf;
                            int pwallposnorm;
                            int pwallposinv;
                            if(*wlitem->pWallDir() == di_west) {
                                pwallposnorm = 64-*wlitem->pWallPos();
                                pwallposinv = *wlitem->pWallPos();
                            } else {
                                pwallposnorm = *wlitem->pWallPos();
                                pwallposinv = 64-*wlitem->pWallPos();
                            }
                            if((*wlitem->pWallDir() == di_east && xtile== *wlitem->pWallX() && ((quint32)yintercept>>16)== *wlitem->pWallY())
                                    || (*wlitem->pWallDir() == di_west && !(xtile== *wlitem->pWallX() && ((quint32)yintercept>>16)== *wlitem->pWallY()))) {
                                yintbuf=yintercept+((ystep*pwallposnorm)>>6);
                                if((yintbuf>>16)!=(yintercept>>16))
                                    goto passvert;

                                xintercept=(xtile<<TILESHIFT)+TILEGLOBAL-(pwallposinv<<10);
                                yintercept=yintbuf;
                                tilehit=*wlitem->pWallTile();
                                HitVertWall();
                            } else {
                                yintbuf=yintercept+((ystep*pwallposinv)>>6);
                                if((yintbuf>>16)!=(yintercept>>16))
                                    goto passvert;

                                xintercept=(xtile<<TILESHIFT)-(pwallposinv<<10);
                                yintercept=yintbuf;
                                tilehit=*wlitem->pWallTile();
                                HitVertWall();
                            }
                        } else {
                            int pwallposi = *wlitem->pWallPos();
                            if(*wlitem->pWallDir() == di_north) pwallposi = 64-*wlitem->pWallPos();
                            if((*wlitem->pWallDir() == di_south && (quint16)yintercept<(pwallposi<<10))
                                    || (*wlitem->pWallDir() == di_north && (quint16)yintercept>(pwallposi<<10))) {
                                if(((quint32)yintercept>>16)== *wlitem->pWallY() && xtile== *wlitem->pWallX()) {
                                    if((*wlitem->pWallDir() == di_south && (qint32)((quint16)yintercept)+ystep<(pwallposi<<10))
                                            || (*wlitem->pWallDir() == di_north && (qint32)((quint16)yintercept)+ystep>(pwallposi<<10)))
                                        goto passvert;

                                    if(*wlitem->pWallDir() == di_south)
                                        yintercept=(yintercept&0xffff0000)+(pwallposi<<10);
                                    else
                                        yintercept=(yintercept&0xffff0000)-TILEGLOBAL+(pwallposi<<10);
                                    xintercept=xintercept-((xstep*(64-*wlitem->pWallPos()))>>6);
                                    tilehit=*wlitem->pWallTile();
                                    HitHorizWall();
                                } else {
                                    texdelta = -(pwallposi<<10);
                                    xintercept=xtile<<TILESHIFT;
                                    tilehit=*wlitem->pWallTile();
                                    HitVertWall();
                                }
                            } else {
                                if(((quint32)yintercept>>16)== *wlitem->pWallY() && xtile== *wlitem->pWallX()) {
                                    texdelta = -(pwallposi<<10);
                                    xintercept=xtile<<TILESHIFT;
                                    tilehit=*wlitem->pWallTile();
                                    HitVertWall();
                                } else {
                                    if((*wlitem->pWallDir() == di_south && (qint32)((quint16)yintercept)+ystep>(pwallposi<<10))
                                            || (*wlitem->pWallDir() == di_north && (qint32)((quint16)yintercept)+ystep<(pwallposi<<10)))
                                        goto passvert;

                                    if(*wlitem->pWallDir() == di_south)
                                        yintercept=(yintercept&0xffff0000)-((64-*wlitem->pWallPos())<<10);
                                    else
                                        yintercept=(yintercept&0xffff0000)+((64-*wlitem->pWallPos())<<10);
                                    xintercept=xintercept-((xstep**wlitem->pWallPos())>>6);
                                    tilehit=*wlitem->pWallTile();
                                    HitHorizWall();
                                }
                            }
                        }
                    } else {
                        xintercept=xtile<<TILESHIFT;
                        HitVertWall();
                    }
                }
                break;
            }
            passvert:
                *((quint8*)spotvis+xspot)=1;
            xtile+=xtilestep;
            yintercept+=ystep;
            xspot=(quint16)((xtile<<mapshift)+((quint32)yintercept>>16));
        }
        while(1);
        continue;

        do {
            if(xtilestep==-1 && (xintercept>>16)<=xtile) goto vertentry;
            if(xtilestep==1 && (xintercept>>16)>=xtile) goto vertentry;
            horizentry:
                if((quint32)xintercept>mapwidth*65536-1 || (quint16)ytile>=mapheight) {
                if(ytile<0) yintercept=0;
                if(ytile>=mapheight) yintercept=mapheight<<TILESHIFT;
                if(xintercept<0) xintercept=0;
                if(xintercept>=(mapwidth<<TILESHIFT)) xintercept=mapwidth<<TILESHIFT;
                xspot=0xffff;
                tilehit=0;
                HitVertBorder();
                break;
            }
            if(yspot>=maparea) break;
            tilehit=((quint8*)tilemap)[yspot];
            if(tilehit) {
                if(tilehit&0x80) {
                    qint32 xintbuf=xintercept+(xstep>>1);
                    if((xintbuf>>16)!=(xintercept>>16))
                        goto passhoriz;
                    if((quint16)xintbuf < *wlitem->doorPosition(tilehit&0x7f))
                        goto passhoriz;
                    xintercept=xintbuf;
                    yintercept=(ytile<<TILESHIFT)+0x8000;
                    HitHorizDoor();
                } else {
                    if(tilehit==64) {
                        if(*wlitem->pWallDir() == di_north || *wlitem->pWallDir() == di_south) {
                            qint32 xintbuf;
                            int pwallposnorm;
                            int pwallposinv;
                            if(*wlitem->pWallDir() == di_north) {
                                pwallposnorm = 64-*wlitem->pWallPos();
                                pwallposinv = *wlitem->pWallPos();
                            } else {
                                pwallposnorm = *wlitem->pWallPos();
                                pwallposinv = 64-*wlitem->pWallPos();
                            }
                            if((*wlitem->pWallDir() == di_south && ytile== *wlitem->pWallY() && ((quint32)xintercept>>16)== *wlitem->pWallX())
                                    || (*wlitem->pWallDir() == di_north && !(ytile== *wlitem->pWallY() && ((quint32)xintercept>>16)== *wlitem->pWallX()))) {
                                xintbuf=xintercept+((xstep*pwallposnorm)>>6);
                                if((xintbuf>>16)!=(xintercept>>16))
                                    goto passhoriz;

                                yintercept=(ytile<<TILESHIFT)+TILEGLOBAL-(pwallposinv<<10);
                                xintercept=xintbuf;
                                tilehit=*wlitem->pWallTile();
                                HitHorizWall();
                            } else {
                                xintbuf=xintercept+((xstep*pwallposinv)>>6);
                                if((xintbuf>>16)!=(xintercept>>16))
                                    goto passhoriz;

                                yintercept=(ytile<<TILESHIFT)-(pwallposinv<<10);
                                xintercept=xintbuf;
                                tilehit=*wlitem->pWallTile();
                                HitHorizWall();
                            }
                        } else {
                            int pwallposi = *wlitem->pWallPos();
                            if(*wlitem->pWallDir() == di_west) pwallposi = 64-*wlitem->pWallPos();
                            if((*wlitem->pWallDir() == di_east && (quint16)xintercept<(pwallposi<<10))
                                    || (*wlitem->pWallDir() == di_west && (quint16)xintercept>(pwallposi<<10))) {
                                if(((quint32)xintercept>>16)== *wlitem->pWallX() && ytile== *wlitem->pWallY()) {
                                    if((*wlitem->pWallDir() == di_east && (qint32)((quint16)xintercept)+xstep<(pwallposi<<10))
                                            || (*wlitem->pWallDir() == di_west && (qint32)((quint16)xintercept)+xstep>(pwallposi<<10)))
                                        goto passhoriz;

                                    if(*wlitem->pWallDir() == di_east)
                                        xintercept=(xintercept&0xffff0000)+(pwallposi<<10);
                                    else
                                        xintercept=(xintercept&0xffff0000)-TILEGLOBAL+(pwallposi<<10);
                                    yintercept=yintercept-((ystep*(64-*wlitem->pWallPos()))>>6);
                                    tilehit=*wlitem->pWallTile();
                                    HitVertWall();
                                } else {
                                    texdelta = -(pwallposi<<10);
                                    yintercept=ytile<<TILESHIFT;
                                    tilehit=*wlitem->pWallTile();
                                    HitHorizWall();
                                }
                            } else {
                                if(((quint32)xintercept>>16)== *wlitem->pWallX() && ytile== *wlitem->pWallY()) {
                                    texdelta = -(pwallposi<<10);
                                    yintercept=ytile<<TILESHIFT;
                                    tilehit=*wlitem->pWallTile();
                                    HitHorizWall();
                                } else {
                                    if((*wlitem->pWallDir() == di_east && (qint32)((quint16)xintercept)+xstep>(pwallposi<<10))
                                            || (*wlitem->pWallDir() ==di_west && (qint32)((quint16)xintercept)+xstep<(pwallposi<<10)))
                                        goto passhoriz;

                                    if(*wlitem->pWallDir() ==di_east)
                                        xintercept=(xintercept&0xffff0000)-((64-*wlitem->pWallPos())<<10);
                                    else
                                        xintercept=(xintercept&0xffff0000)+((64-*wlitem->pWallPos())<<10);
                                    yintercept=yintercept-((ystep**wlitem->pWallPos())>>6);
                                    tilehit=*wlitem->pWallTile();
                                    HitVertWall();
                                }
                            }
                        }
                    } else {
                        yintercept=ytile<<TILESHIFT;
                        HitHorizWall();
                    }
                }
                break;
            }
            passhoriz:
                *((quint8*)spotvis+yspot)=1;
            ytile+=ytilestep;
            xintercept+=xstep;
            yspot=(quint16)((((quint32)xintercept>>16)<<mapshift)+ytile);
        }
        while(1);
    }
}

void WallRefresh (void)
{
    xpartialdown = viewx&(TILEGLOBAL-1);
    xpartialup = TILEGLOBAL-xpartialdown;
    ypartialdown = viewy&(TILEGLOBAL-1);
    ypartialup = TILEGLOBAL-ypartialdown;
    min_wallheight = viewheight;
    lastside = -1;                  // the first pixel is on a new wall
    AsmRefresh ();
    ScalePost ();                   // no more optimization on last post
}

void CalcViewVariables()
{
    viewangle = player->angle;
    midangle = viewangle*(FINEANGLES/ANGLES);
    viewsin = sintable[viewangle];
    viewcos = costable[viewangle];
    viewx = player->x - FixedMul(focallength,viewcos);
    viewy = player->y + FixedMul(focallength,viewsin);
    focaltx = (short)(viewx>>TILESHIFT);
    focalty = (short)(viewy>>TILESHIFT);
    viewtx = (short)(player->x >> TILESHIFT);
    viewty = (short)(player->y >> TILESHIFT);
}

void  ThreeDRefresh (void)
{
    memset(spotvis,0,maparea);
    spotvis[player->tilex][player->tiley] = 1;

    if (g_Image) {
        vbuf = g_Image->bits();
        vbuf+=screenofs;
        vbufPitch = g_Image->bytesPerLine();
        CalcViewVariables();
        VGAClearScreen ();
        WallRefresh ();
        DrawScaleds();
        DrawPlayerWeapon ();
        if(wlinput->isKeyDown(sc_Tab) && viewsize == 21 && gamestate.weapon != -1)
            ShowActStatus();
    }
    vbuf = NULL;

    if (fizzlein) {
        fizzlein = false;

        lasttimecount = g_getTicks->elapsed()*7/100;          // don't make a big tic count
    } else {
        if (fpscounter) {
            fontnumber = 0;
            SETFONTCOLOR(7,127);
            PrintX=4; PrintY=1;
            wlpaint->bar(0, 0, 50, 10, bordercol);
            wluser->US_PrintSigned(fps);
            wluser->US_Print(" fps");
        }
    }
    if (fpscounter) {
        fps_frames++;
        fps_time+=tics;
        if(fps_time>35) {
            fps_time-=35;
            fps=fps_frames<<1;
            fps_frames=0;
        }
    }
    wlpaint->updateScreen();
}
