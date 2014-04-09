/****************************************************************************
 *
 * class PolyBuilder
 *
 * Proprietary software product of Pointwise, Inc.
 * Copyright (c) 1995-2011 Pointwise, Inc.
 * All rights reserved.
 *
 ***************************************************************************/

#ifndef _POLYBUILDER_H_
#define _POLYBUILDER_H_

#include "apiPWP.h"
#include "cml.h"
#include "Polygon.h"
#include "VertElemRange.h"

class CaeUnsGridModel;


//***************************************************************************
//***************************************************************************
//***************************************************************************

class PolyBuilder {
public:

    PolyBuilder(const CaeUnsGridModel &model) :
        model_(model)
    {
        reset();
    }

    ~PolyBuilder() {
    }

    void reset(PWP_UINT32 numSides = 0) {
        minRSquared_ = 1e8;
        maxRSquared_ = 0.0;
        quality_ = -1.0;
        poly().clear(numSides);
    }

    const Polygon& poly() const {
        return poly_;
    }

    Polygon& poly() {
        return poly_;
    }

    bool        build(const VertElemRange &rng);

    bool        polyIsConvex() const;

    PWP_FLOAT   quality() const;

    PWP_FLOAT   minRSquared() const {
                    return minRSquared_;
                }

    PWP_FLOAT   maxRSquared() const {
                    return maxRSquared_;
                }

    static bool vertToVec3(const CaeUnsVertex &vertex, vector3 &v3);


private:

    void        pushRadial(PWP_UINT32 i0, PWP_UINT32 i1);

    void        pushRSquared(PWP_FLOAT rSquared);


private:

    const CaeUnsGridModel &     model_;
    Polygon                     poly_;        // the merged polygon
    PWP_FLOAT                   minRSquared_; // min R^2 value
    PWP_FLOAT                   maxRSquared_; // max R^2 value
    mutable PWP_FLOAT           quality_;     // minRSquared / maxRSquared
};

#endif // _POLYBUILDER_H_
