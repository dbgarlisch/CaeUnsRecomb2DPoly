/****************************************************************************
 *
 * class CaeUnsRecomb2DPoly
 *
 * Proprietary software product of Pointwise, Inc.
 * Copyright (c) 1995-2011 Pointwise, Inc.
 * All rights reserved.
 *
 ***************************************************************************/

#include "apiCAEP.h"
#include "apiCAEPUtils.h"
#include "apiGridModel.h"
#include "apiPWP.h"
#include "runtimeWrite.h"
#include "pwpPlatform.h"
#include "PwpFile.h"

#include "CaePlugin.h"
#include "CaeUnsGridModel.h"
#include "CaeUnsRecomb2DPoly.h"
#include "Polygon.h"
#include "PolyBuilder.h"
#include "VertElemRange.h"


const char *attrDebugDump   = "DebugDump";
const char *attrQuality     = "Quality";
const char *attrMinSides    = "MinSides";
const char *attrMaxSides    = "MaxSides";
const char *attrInlineProcs = "InlineProcs";


//***************************************************************************
//***************************************************************************
//***************************************************************************

CaeUnsRecomb2DPoly::CaeUnsRecomb2DPoly(CAEP_RTITEM *pRti, PWGM_HGRIDMODEL
        model, const CAEP_WRITEINFO *pWriteInfo) :
    CaeUnsPlugin(pRti, model, pWriteInfo),
    dumpFile_(),
    minQuality_(0.0),
    minSides_(0),
    maxSides_(0),
    inlineProcs_(true),
    polyModel_(model_)
{
    bool doDump;
    model_.getAttribute(attrDebugDump, doDump);
    model_.getAttribute(attrQuality, minQuality_);
    model_.getAttribute(attrMinSides, minSides_);
    model_.getAttribute(attrMaxSides, maxSides_);
    model_.getAttribute(attrInlineProcs, inlineProcs_);

    char filename[512];
    strcpy(filename, writeInfo_.fileDest);
    strcat(filename, ".dump");
    if (!doDump) {
        // remove dump file if it exists from previous run
        pwpFileDelete(filename);
    }
    else {
        dumpFile_.open(filename, pwpWrite | pwpAscii);
        sendInfoMsg("debug dump file:", 0);
        sendInfoMsg(filename);
        if (!dumpFile_.isOpen()) {
            sendErrorMsg("debug dump file open failed!", 0);
        }
        else {
            fprintf(dumpFile_, "setParam quality %g\n", minQuality_);
        }
    }

    char msg[512];
    sprintf(msg, "Recombination quality = %g", minQuality_);
    sendInfoMsg(msg, 0);
}


CaeUnsRecomb2DPoly::~CaeUnsRecomb2DPoly()
{
}


bool
CaeUnsRecomb2DPoly::beginExport()
{
    const char *procs = 0;
    if (inlineProcs_) {
        procs =
            "# load with defaults\n"
            "array set params {\n"
            "   minQuality	0.0\n"
            "   minSides	0\n"
            "   maxSides	0\n"
            "}\n"
            "\n"
            "proc setParam { name val } {\n"
            "   global params\n"
            "   set params($name) $val\n"
            "}\n"
            "\n"
            "proc polygon { blk indices } {\n"
            "   global vertices\n"
            "   set len [llength $indices]\n"
            "   if { $len > 2 } {\n"
	        "      # append starting index to wrap poly\n"
	        "      lappend indices [lindex $indices 0]\n"
	        "      # the poly curve\n"
	        "      set poly [pw::Curve create]\n"
	        "      # add poly side segments\n"
	        "      for {set ii 0} {$ii < $len} {} {\n"
	        "         set segment [pw::SegmentSpline create]\n"
	        "         $segment addPoint [lindex $vertices [lindex $indices $ii]]\n"
	        "         $segment addPoint [lindex $vertices [lindex $indices [incr ii]]]\n"
	        "         $poly addSegment $segment\n"
	        "      }\n"
	        "      $poly setRenderAttribute LineWidth 3\n"
	        "      $poly setRenderAttribute ColorMode Entity\n"
	        "      $poly setColor 0x00ffff00\n"
	        "      unset segment poly\n"
            "   }\n"
            "}\n";
    }
    else {
        procs =
            "set scriptDir [file dirname [info script]]\n"
            "source [file join $scriptDir lib {libLoadPolys.glf}]\n";
    }

    fprintf(fp(), "package require PWI_Glyph 2.17.0\n");
    fprintf(fp(), "\n");
    fprintf(fp(), procs);
    fprintf(fp(), "\n");
    fprintf(fp(), "setParam minQuality %g\n", (double)minQuality_);
    fprintf(fp(), "setParam minSides %lu\n", (unsigned long)minSides_);
    fprintf(fp(), "setParam maxSides %lu\n", (unsigned long)maxSides_);
    fprintf(fp(), "\n");

    CaeUnsBlock blk(model_);
    while(blk.isValid()) {
        recombine(blk);
        ++blk;
    }
    return true;
}

PWP_BOOL
CaeUnsRecomb2DPoly::write()
{
    PWP_UINT32 ndx = 0;
    CaeUnsVertex vertex;
    fprintf(fp(), "set vertices {\n");
    while (polyModel_.enumVertices(ndx++, vertex)) {
        fprintf(fp(), "  {%s}\n", vertex.toString());
    }
    fprintf(fp(), "}\n");

    // polygon blk {0 1 2 3 4}
    unsigned long blkId = 0;
    PolyBlockArray::const_iterator itBlk = polyModel_.begin();
    for (; itBlk != polyModel_.end(); ++itBlk, ++blkId) {
        PolygonArray::const_iterator itPoly = itBlk->begin();
        for (; itPoly != itBlk->end(); ++itPoly) {
            fprintf(fp(), "polygon %lu {%s}\n", blkId, itPoly->toString());
        }
    }

    return true;
}

bool
CaeUnsRecomb2DPoly::endExport()
{
    return true;
}

void
CaeUnsRecomb2DPoly::recombine(const CaeUnsBlock & blk)
{
    if (dumpFile_.isOpen()) {
        fprintf(dumpFile_, "##################\n# BLOCK %lu\n"
            "##################\n", (unsigned long)blk.index());
    }
    VertElemMap vertElems;
    CaeUnsElement elem(blk);
    PWGM_ELEMDATA data;
    while(elem.data(data)) {
        for (PWP_UINT32 ii = 0; ii < data.vertCnt; ++ii) {
            blkVerts_.insert(data.index[ii]);
            vertElems.insert(VertElemVal(data.index[ii], elem.index()));
        }
        ++elem;
    }
    PWP_FLOAT avgMinR = 0.0;
    PolygonArray eligiblePolys;
    VertElemRange rng(blk);
    UIntSet::const_iterator bvIt = blkVerts_.begin();
    for (; bvIt != blkVerts_.end(); ++bvIt) {
        // rng.first is iter to range's first valid VertElemMap item.
        // rng.second is iter to the item just AFTER range's last valid
        // VertElemMap item.
        rng.set(vertElems.equal_range(*bvIt));
        if (rng.isEmpty()) {
            continue; // empty range! should never get here!
        }
        PolyBuilder pb(model_);
        PWP_UINT32 elemCnt = (PWP_UINT32)std::distance(rng.first, rng.second);
        if ((elemCnt < minSides_) || (elemCnt > maxSides_)) {
            dumpNoBuildPoly(pb, rng);
            continue; // elemCnt is out of range
        }
        if (!pb.build(rng)) {
            dumpNoBuildPoly(pb, rng);
        }
        else if (pb.quality() < minQuality_) {
            dumpBadQualityPoly(pb, rng);
        }
        else if (!pb.polyIsConvex()) {
            dumpConcavePoly(pb, rng);
        }
        else {
            // poly is eligible for recombination
            eligiblePolys.push_back(pb.poly());
            dumpEligiblePoly(pb, rng);
            avgMinR += pb.minRSquared();
        }
    }
    PolygonArray polyBlock;
    avgMinR /= eligiblePolys.size();
    std::sort(eligiblePolys.begin(), eligiblePolys.end(), qualitySort);
    // indices of recombined polys
    UIntSet usedIndices;
    UIntSet usedElements;
    PolygonArray::iterator polyIt = eligiblePolys.begin();
    for (; polyIt != eligiblePolys.end(); ++polyIt) {
        Polygon &poly = *polyIt;
        if (usedIndices.end() == usedIndices.find(poly.owner())) {
            // The poly owner vertex index has not been part of a previous
            // poly. So we can use this recomb poly.
            polyBlock.push_back(poly);
            // poly's indices are now used
            usedIndices.insert(poly.indices().begin(), poly.indices().end());
            // poly's component tris are now used
            rng.set(vertElems.equal_range(poly.owner()));
            VertElemMap::const_iterator veIt = rng.first;
            VertElemMap::const_iterator veEndIt = rng.second;
            for (; veIt != veEndIt; ++veIt) { // elemId0 ... elemIdN
                usedElements.insert(veIt->second);
            }
        }
    }
    // run thru blk and add elements not in usedElements to polyBlock
    elem.moveFirst(blk);
    while(elem.isValid()) {
        if (usedElements.end() != usedElements.find(elem.index())) {
            // elem was part of a recombined poly - skip it
        }
        else if (elem.data(data)) {
            polyBlock.push_back(Polygon(data.index, data.vertCnt));
        }
        ++elem;
    }
    dumpRecombPolys(polyBlock, avgMinR);
    // this call will modify polyBlock changing poly indices from the original
    // global model_ indices to polyModel_ indices
    polyModel_.push_back(polyBlock);
}

bool
CaeUnsRecomb2DPoly::qualitySort(const Polygon &poly1, const Polygon &poly2)
{
    return poly1.quality() > poly2.quality();
}

void
CaeUnsRecomb2DPoly::dumpLabelPt(const char *lbl, const CaeUnsVertex &vert,
    PWP_UINT32 color, PWP_UINT32 rotAngle) const
{
    if (dumpFile_.isOpen()) {
        // write call to tcl proc
        //
        //   labelPt lbl pt color color rotAngle
        //
        fprintf(dumpFile_, "labelPt \"%s\" {%s} 0x%lX %lu\n", lbl,
            vert.toString(), color, rotAngle);
    }
}

const PWP_UINT32 WHITE  = 0x00FFFFFF;
const PWP_UINT32 RED    = 0x00FF0000;
const PWP_UINT32 GREEN  = 0x0000FF00;
const PWP_UINT32 BLUE   = 0x000000FF;
const PWP_UINT32 YELLOW = 0x00FFFF00;
const PWP_UINT32 ORANGE = 0x00FFA500;


void
CaeUnsRecomb2DPoly::dumpNoBuildPoly(const PolyBuilder &pb, const VertElemRange
    &rng) const
{
    (void)&pb; // silence unused warning
    char lbl[1024];
    // lbl = "blkId@vertId\nBAD"
    sprintf(lbl, "%lu@%lu\\nBAD", (unsigned long)rng.block().index(),
        (unsigned long)rng.first->first);
    dumpLabelPt(lbl, CaeUnsVertex(model_, rng.first->first), RED);
}

void
CaeUnsRecomb2DPoly::dumpBadQualityPoly(const PolyBuilder &pb,
    const VertElemRange &rng) const
{
    char lbl[1024];
    // lbl = "quality\nblkId@vertId"
    sprintf(lbl, "%4.3g\\n%lu@%lu", pb.quality(),
        (unsigned long)rng.block().index(), (unsigned long)rng.first->first);
    dumpLabelPt(lbl, CaeUnsVertex(model_, rng.first->first), YELLOW);
}

void
CaeUnsRecomb2DPoly::dumpConcavePoly(const PolyBuilder &pb, const VertElemRange
    &rng) const
{
    (void)&pb; // silence unused warning
    char lbl[1024];
    // lbl = "blkId@vertId\nBAD"
    sprintf(lbl, "%lu@%lu\\nCONCAVE", (unsigned long)rng.block().index(),
        (unsigned long)rng.first->first);
    dumpLabelPt(lbl, CaeUnsVertex(model_, rng.first->first), BLUE);
}

void
CaeUnsRecomb2DPoly::dumpEligiblePoly(const PolyBuilder &pb, const VertElemRange
    &rng) const
{
    // rng.first is iter to range's first valid VertElemMap item. rng.second
    // is iter to the item just AFTER range's last valid VertElemMap item.
    VertElemMap::const_iterator veIt = rng.first;
    VertElemMap::const_iterator veEndIt = rng.second;
    // veIt->first == model's vertIndex
    // veIt->second == blk's elemIndex
    //
    PWP_UINT32 elemCnt = (PWP_UINT32)std::distance(veIt, veEndIt);
    CaeUnsVertex vert(model_, veIt->first);
    char lbl[1024];
    char *p = lbl;
    // lbl = "quality\nblkId@vertId\nelemCnt\[elemId0 ... elemIdN\]"
    p += sprintf(p, "%4.3g\\n%lu@%lu\\n%lu\\[", pb.quality(),
            (unsigned long)rng.block().index(), (unsigned long)vert.index(),
            (unsigned long)elemCnt);
    for (; veIt != veEndIt; ++veIt) { // elemId0 ... elemIdN
        p += sprintf(p, "%lu ", (unsigned long)veIt->second);
    }
    // -1 to overwrite trailing blank
    sprintf(p - 1, "\\]");
    dumpLabelPt(lbl, vert, GREEN);
}

static const char *
polyToString(const CaeUnsGridModel &model, const Polygon &poly)
{
    static char polyStrBuf[1024];
    polyStrBuf[1] = '\0'; // just in case poly is empty
    char *p = polyStrBuf;
    const UIntArray &indices = poly.indices();
    UIntArray::const_iterator it = indices.begin();
    for(; it != indices.end(); ++it) {
        CaeUnsVertex vert(model, *it);
        p += sprintf(p, " {%s}", vert.toString());
    }
    return polyStrBuf + 1;
}

void
CaeUnsRecomb2DPoly::dumpRecombPolys(const PolygonArray & polyBlock,
    PWP_FLOAT avgMinR) const
{
    if (dumpFile_.isOpen()) {
        PolygonArray::const_iterator it = polyBlock.begin();
        for (; it != polyBlock.end(); ++it) {
            if (it->isUnowned()) {
                fprintf(dumpFile_, "poly {%s}\n",
                    polyToString(model_, *it));
            }
            else {
                CaeUnsVertex vert(model_, it->owner());
                fprintf(dumpFile_, "circle {%s} %g\n", vert.toString(),
                    avgMinR);
            }
        }
    }
}


//===========================================================================
// face streaming handlers
//===========================================================================

PWP_UINT32
CaeUnsRecomb2DPoly::streamBegin(const PWGM_BEGINSTREAM_DATA &data)
{
    char msg[128];
    sprintf(msg, "STREAM BEGIN: %lu", (unsigned long)data.totalNumFaces);
    sendInfoMsg(msg);
    return 1;
}

PWP_UINT32
CaeUnsRecomb2DPoly::streamFace(const PWGM_FACESTREAM_DATA &data)
{
    char msg[128];
    sprintf(msg, "  STREAM FACE: %lu %lu", (unsigned long)data.owner.cellIndex,
        (unsigned long)data.face);
    sendInfoMsg(msg);
    return 1;
}

PWP_UINT32
CaeUnsRecomb2DPoly::streamEnd(const PWGM_ENDSTREAM_DATA &data)
{
    char msg[128];
    sprintf(msg, "STREAM END: %s", (data.ok ? "true" : "false"));
    sendInfoMsg(msg);
    return 1;
}


//===========================================================================
// called ONCE when plugin first loaded into memeory
//===========================================================================

bool
CaeUnsRecomb2DPoly::create(CAEP_RTITEM &rti)
{
    (void)rti.BCCnt; // silence unused arg warning
    return
        publishRealValueDef(rti, attrQuality, 0.8,
            "Recombination quality threshold", 0.0, 1.0, 0.7, 1.0) &&

        publishUIntValueDef(rti, attrMinSides, 4,
            "Minimum sides in the recombined poly", 4, 100, 4, 12) &&

        publishUIntValueDef(rti, attrMaxSides, 100,
            "Maximum sides in the recombined poly", 4, 100, 4, 12) &&

        publishBoolValueDef(rti, attrDebugDump, true,
            "Generate a debug dump file?", "no|yes") &&

        publishBoolValueDef(rti, attrInlineProcs, true,
            "Inline the glyph script procs", "no|yes");
}


//===========================================================================
// called ONCE just before plugin unloaded from memeory
//===========================================================================

void
CaeUnsRecomb2DPoly::destroy(CAEP_RTITEM &rti)
{
    (void)rti.BCCnt; // silence unused arg warning
}
