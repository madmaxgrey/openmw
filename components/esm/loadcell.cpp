#include "loadcell.hpp"

#include <string>
#include <sstream>

namespace ESM
{

void CellRef::save(ESMWriter &esm)
{
    esm.writeHNT("FRMR", refnum);
    esm.writeHNString("NAME", refID);

    if (scale != 1.0)
        esm.writeHNT("XSCL", scale);

    esm.writeHNOString("ANAM", owner);
    esm.writeHNOString("BNAM", glob);
    esm.writeHNOString("XSOL", soul);

    esm.writeHNOString("CNAM", faction);
    if (factIndex != -1)
        esm.writeHNT("INDX", factIndex);

    if (charge != -1.0)
        esm.writeHNT("XCHG", charge);

    if (intv != 0)
        esm.writeHNT("INTV", intv);
    if (nam9 != 0)
        esm.writeHNT("NAM9", nam9);

    if (teleport)
    {
        esm.writeHNT("DODT", doorDest);
        esm.writeHNOString("DNAM", destCell);
    }

    if (lockLevel != 0)
        esm.writeHNT("FLTV", lockLevel);
    esm.writeHNOString("KNAM", key);
    esm.writeHNOString("TNAM", trap);

    if (unam != 0)
        esm.writeHNT("UNAM", unam);
    if (fltv != 0)
        esm.writeHNT("FLTV", fltv);

    esm.writeHNT("DATA", pos, 24);
}

void Cell::load(ESMReader &esm)
{
    // Ignore this for now, it might mean we should delete the entire
    // cell?
    if (esm.isNextSub("DELE"))
        esm.skipHSub();

    esm.getHNT(data, "DATA", 12);

    // Water level
    water = 0;

    if (data.flags & Interior)
    {
        // Interior cells
        if (esm.isNextSub("INTV"))
        {
            int waterl;
            esm.getHT(waterl);
            water = (float) waterl;
        }
        else if (esm.isNextSub("WHGT"))
            esm.getHT(water);

        // Quasi-exterior cells have a region (which determines the
        // weather), pure interior cells have ambient lighting
        // instead.
        if (data.flags & QuasiEx)
            region = esm.getHNOString("RGNN");
        else
            esm.getHNT(ambi, "AMBI", 16);
    }
    else
    {
        // Exterior cells
        region = esm.getHNOString("RGNN");
        esm.getHNOT(mapColor, "NAM5");
    }

    // Save position of the cell references and move on
    context = esm.getContext();
    esm.skipRecord();
}

void Cell::save(ESMWriter &esm)
{
    esm.writeHNT("DATA", data, 12);
    if (data.flags & Interior)
    {
        if (water != 0)
            esm.writeHNT("WHGT", water);

        if (data.flags & QuasiEx)
            esm.writeHNOString("RGNN", region);
        else
            esm.writeHNT("AMBI", ambi, 16);
    }
    else
    {
        esm.writeHNOString("RGNN", region);
        if (mapColor != 0)
            esm.writeHNT("NAM5", mapColor);
    }
}

void Cell::restore(ESMReader &esm) const
{
    esm.restoreContext(context);
}

std::string Cell::getDescription() const
{
    if (data.flags & Interior)
    {
        return name;
    }
    else
    {
        std::ostringstream stream;
        stream << data.gridX << ", " << data.gridY;
        return stream.str();
    }
}

bool Cell::getNextRef(ESMReader &esm, CellRef &ref)
{
    if (!esm.hasMoreSubs())
        return false;

    // Number of references in the cell? Maximum once in each cell,
    // but not always at the beginning, and not always right. In other
    // words, completely useless.
    {
        int i;
        esm.getHNOT(i, "NAM0");
    }

    esm.getHNT(ref.refnum, "FRMR");
    ref.refID = esm.getHNString("NAME");

    // getHNOT will not change the existing value if the subrecord is
    // missing
    ref.scale = 1.0;
    esm.getHNOT(ref.scale, "XSCL");

    ref.owner = esm.getHNOString("ANAM");
    ref.glob = esm.getHNOString("BNAM");
    ref.soul = esm.getHNOString("XSOL");

    ref.faction = esm.getHNOString("CNAM");
    ref.factIndex = -1;
    esm.getHNOT(ref.factIndex, "INDX");

    ref.charge = -1.0;
    esm.getHNOT(ref.charge, "XCHG");

    ref.intv = 0;
    ref.nam9 = 0;
    esm.getHNOT(ref.intv, "INTV");
    esm.getHNOT(ref.nam9, "NAM9");

    // Present for doors that teleport you to another cell.
    if (esm.isNextSub("DODT"))
    {
        ref.teleport = true;
        esm.getHT(ref.doorDest);
        ref.destCell = esm.getHNOString("DNAM");
    }
    else
        ref.teleport = false;

    // Integer, despite the name suggesting otherwise
    ref.lockLevel = 0;
    esm.getHNOT(ref.lockLevel, "FLTV");
    ref.key = esm.getHNOString("KNAM");
    ref.trap = esm.getHNOString("TNAM");

    ref.unam = 0;
    ref.fltv = 0;
    esm.getHNOT(ref.unam, "UNAM");
    esm.getHNOT(ref.fltv, "FLTV");

    esm.getHNT(ref.pos, "DATA", 24);

    return true;
}

}
