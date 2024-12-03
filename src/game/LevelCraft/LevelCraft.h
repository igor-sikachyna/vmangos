// LevelCraft

#ifndef MANGOSSERVER_LEVELCRAFT_H
#define MANGOSSERVER_LEVELCRAFT_H

class LevelCraft
{
public:
    LevelCraft();

    virtual bool Create();

    virtual void SaveToDB();
    virtual bool LoadFromDB();
private:
};

#endif
