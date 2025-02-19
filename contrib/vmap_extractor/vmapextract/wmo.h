/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef WMO_H
#define WMO_H
#define TILESIZE (533.33333f)
#define CHUNKSIZE ((TILESIZE) / 16.0f)

#include <string>
#include <set>
#include <memory>
#include "vec3d.h"
#include "loadlib/loadlib.h"
#include <unordered_set>
#include "adtfile.h"

// MOPY flags
enum MopyFlags
{
    WMO_MATERIAL_UNK01          = 0x01,
    WMO_MATERIAL_NOCAMCOLLIDE   = 0x02,
    WMO_MATERIAL_DETAIL         = 0x04,
    WMO_MATERIAL_COLLISION      = 0x08,
    WMO_MATERIAL_HINT           = 0x10,
    WMO_MATERIAL_RENDER         = 0x20,
    WMO_MATERIAL_WALL_SURFACE   = 0x40, // Guessed
    WMO_MATERIAL_COLLIDE_HIT    = 0x80
};

class WMOManager;
class MPQFile;

namespace WMO
{
    struct MODS
    {
        char Name[20];
        uint32 StartIndex;     // index of first doodad instance in this set
        uint32 Count;          // number of doodad instances in this set
        char _pad[4];
    };

    struct MODD
    {
        uint32 NameIndex : 24;
        Vec3D Position;
        Quaternion Rotation;
        float Scale;
        uint32 Color;
    };
}

/* for whatever reason a certain company just can't stick to one coordinate system... */
static inline Vec3D fixCoords(const Vec3D& v) { return Vec3D(v.z, v.x, v.y); }

struct WMODoodadData
{
    std::vector<WMO::MODS> Sets;
    std::unique_ptr<char[]> Paths;
    std::vector<WMO::MODD> Spawns;
    std::unordered_set<uint16> References;
};

class WMORoot
{
    public:
        unsigned int color;
        uint32 nTextures, nGroups, nPortals, nLights, nDoodadNames, nDoodadDefs, nDoodadSets, RootWMOID, flags;
        float bbcorn1[3];
        float bbcorn2[3];

        WMODoodadData DoodadData;
        std::unordered_set<uint32> ValidDoodadNames;
        std::vector<char> GroupNames;

        WMORoot(std::string& filename);
        ~WMORoot();

        bool open();
        bool ConvertToVMAPRootWmo(FILE* output);
    private:
        std::string filename;
        char outfilename;
};

struct WMOLiquidHeader
{
    int32 xverts, yverts, xtiles, ytiles;
    float pos_x;
    float pos_y;
    float pos_z;
    uint16 type;
};

constexpr int32 WMOLiquidHeaderSize = sizeof(int32) /* xverts */ + sizeof(int32) /* yverts */ + sizeof(int32) /* xtiles */ + sizeof(int32) /* ytiles */ + sizeof(float) /* pos_x */ + sizeof(float) /* pos_y */ + sizeof(float) /* pos_z */ + sizeof(uint16) /* type */;

struct WMOLiquidVert
{
    uint16 unk1;
    uint16 unk2;
    float height;
};

class WMOGroup
{
    public:
        // MOGP
        int groupName, descGroupName, mogpFlags;
        float bbcorn1[3];
        float bbcorn2[3];
        uint16 moprIdx;
        uint16 moprNItems;
        uint16 nBatchA;
        uint16 nBatchB;
        uint32 nBatchC, fogIdx, liquidType, groupWMOID;

        int mopy_size, moba_size;
        int LiquEx_size;
        unsigned int nVertices; // number when loaded
        int nTriangles; // number when loaded
        char* MOPY;
        uint16* MOVI;
        uint16* MoviEx;
        float* MOVT;
        uint16* MOBA;
        int* MobaEx;
        WMOLiquidHeader* hlq;
        WMOLiquidVert* LiquEx;
        char* LiquBytes;
        uint32 liquflags;

        std::vector<uint16> DoodadReferences;

        WMOGroup(std::string& filename);
        ~WMOGroup();

        bool open();
        int ConvertToVMAPGroupWmo(FILE* output, WMORoot* rootWMO, bool pPreciseVectorData);

        bool ShouldSkip(WMORoot const& root) const;

    private:
        std::string filename;
        char outfilename;
};

class WMOInstance
{
        static std::set<int> ids;
    public:
        ADT::MODF m_wmo;

        WMOInstance(MPQFile& f, const char* WmoInstName, uint32 mapID, uint32 tileX, uint32 tileY, FILE* pDirfile);

        static void reset();
};

#endif
