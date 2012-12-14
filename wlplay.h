#ifndef WLPLAY_H
#define WLPLAY_H

class WLPlay {
public:
    WLPlay();
    ~WLPlay();

    qint32 stopMusic();
    void centerWindow(quint16 w, quint16 h);
    void initObjList();
    void removeObj(objtype * gone);
    void pollControls();
    void startMusic();
    void continueMusic(qint32 offs);
    void playLoop();
    void pollKeyboardButtons();
    void pollKeyboardMove();
    void checkKeys();
    void initActorList();
    void getNewActor();
    void initRedShifts();
    void clearPaletteShifts();
    void startBonusFlash();
    void startDamageFlash(qint32 damage);
    void updatePaletteShifts();
    void finishPaletteShifts();
    void doActor(objtype * ob);
    void screenShot();
    void showMap();
};

#endif // WLPLAY_H
