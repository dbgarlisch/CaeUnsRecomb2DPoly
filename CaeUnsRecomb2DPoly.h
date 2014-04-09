/****************************************************************************
 *
 * class CaeUnsRecomb2DPoly
 *
 * Proprietary software product of Pointwise, Inc.
 * Copyright (c) 1995-2011 Pointwise, Inc.
 * All rights reserved.
 *
 ***************************************************************************/

#ifndef _CAEUNSRECOMB2DPOLY_H_
#define _CAEUNSRECOMB2DPOLY_H_

#include <unordered_map>
#include <unordered_set>
#include <map>
#include <utility>
#include <vector>

#include "apiGridModel.h"
#include "apiPWP.h"
#include "apiCAEP.h"
#include "apiCAEPUtils.h"

#include "CaePlugin.h"
#include "CaeUnsGridModel.h"
#include "cml.h"
#include "Polygon.h"
#include "PwpFile.h"
#include "VertElemRange.h"


typedef STDC11::unordered_multimap<PWP_UINT32, PWP_UINT32> UIntUIntMultiMap;

typedef UIntUIntMultiMap                    VertElemMap;
typedef VertElemMap::value_type             VertElemVal;
typedef STDC11::unordered_set<PWP_UINT32>   UIntSet;
typedef std::vector<PWP_UINT32>             UIntVec;
typedef std::vector<PolygonArray>           PolyBlockArray;

typedef std::map<PWP_UINT32, PWP_UINT32>        UIntUIntMap;
typedef UIntUIntMap::value_type                 UIntUIntMapVal;
typedef std::pair<UIntUIntMap::iterator, bool>  UIntUIntPair;

class PolyBuilder;


//***************************************************************************
//***************************************************************************
//***************************************************************************

class RecombPolyModel {
public:

    RecombPolyModel(const CaeUnsGridModel &globalModel) :
        globalModel_(globalModel)
    {
    }

    ~RecombPolyModel() {
    }

    void push_back(PolygonArray &polys) {
        // process poly indices
        PolygonArray::iterator itPoly = polys.begin();
        for (; itPoly != polys.end(); ++itPoly) {
            UIntArray &polyIndices = itPoly->indices();
            // iterate poly's indices. Initially global, each is replaced with
            // the corresponding local index.
            UIntArray::iterator itNdx = polyIndices.begin();
            for (; itNdx != polyIndices.end(); ++itNdx) {
                // insert (global, local) mapping pair
                UIntUIntPair ret = indexMap_.insert(UIntUIntMapVal(*itNdx,
                                        (PWP_UINT32)indices_.size()));
                // if indexMap_.insert() inserted a new value
                if (ret.second) {
                    // append global index to local indices array
                    indices_.push_back(*itNdx);
                }
                // replace global poly index with local index
                //UIntUIntMapVal &mapVal = *ret.first;
                *itNdx = ret.first->second;
            }
        }
        // all polys now have local indices
        blocks_.push_back(polys);
    }

    PolyBlockArray::const_iterator begin() const {
        return blocks_.begin();
    }

    PolyBlockArray::const_iterator end() const {
        return blocks_.end();
    }

    PWP_UINT32 blockCount() const {
        return (PWP_UINT32)blocks_.size();
    }

    PWP_UINT32 vertexCount() const {
        return (PWP_UINT32)indices_.size();
    }

    bool enumVertices(PWP_UINT32 locNdx, CaeUnsVertex &vertex)
        const
    {
        return (locNdx < indices_.size()) ?
            vertex.moveTo(globalModel_, indices_.at(locNdx)).isValid() : false;
    }

private:
    const CaeUnsGridModel & globalModel_; // the original unrecombined model
    PolyBlockArray          blocks_;      // blocks with recombined elements
    UIntUIntMap             indexMap_;    // maps global index to local index
    UIntArray               indices_;     // global indices in local order
};


//***************************************************************************
//***************************************************************************
//***************************************************************************

class CaeUnsRecomb2DPoly : public CaeUnsPlugin, public CaeFaceStreamHandler {

public:
    CaeUnsRecomb2DPoly(CAEP_RTITEM *pRti, PWGM_HGRIDMODEL model,
        const CAEP_WRITEINFO *pWriteInfo);

    ~CaeUnsRecomb2DPoly();

    static bool create(CAEP_RTITEM &rti);
    static void destroy(CAEP_RTITEM &rti);

private:

    void    recombine(const CaeUnsBlock &blk);

    static bool qualitySort(const Polygon &poly1, const Polygon &poly2);

    void    dumpLabelPt(const char *lbl, const CaeUnsVertex &vert,
                PWP_UINT32 color = 0x00FFFFFF, PWP_UINT32 rotAngle = 0) const;

    void    dumpNoBuildPoly(const PolyBuilder &pb, const VertElemRange &rng)
                const;

    void    dumpBadQualityPoly(const PolyBuilder &pb, const VertElemRange &rng)
                const;

    void    dumpConcavePoly(const PolyBuilder &pb, const VertElemRange &rng)
                const;

    void    dumpEligiblePoly(const PolyBuilder &pb, const VertElemRange &rng)
                const;

    void    dumpRecombPolys(const PolygonArray & polyBlock, PWP_FLOAT avgMinR) const;


private: // base class virtual methods

    virtual bool        beginExport();
    virtual PWP_BOOL    write();
    virtual bool        endExport();

    // face streaming handlers
    virtual PWP_UINT32 streamBegin(const PWGM_BEGINSTREAM_DATA &data);
    virtual PWP_UINT32 streamFace(const PWGM_FACESTREAM_DATA &data);
    virtual PWP_UINT32 streamEnd(const PWGM_ENDSTREAM_DATA &data);

private:

    mutable PwpFile dumpFile_;
    PWP_FLOAT       minQuality_;
    PWP_UINT32      minSides_;
    PWP_UINT32      maxSides_;
    bool            inlineProcs_;
    UIntSet         blkVerts_;
    RecombPolyModel polyModel_;
};

#endif // _CAEUNSRECOMB2DPOLY_H_
