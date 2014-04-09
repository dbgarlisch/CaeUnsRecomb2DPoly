/****************************************************************************
 *
 * class PolyBuilder
 *
 * Proprietary software product of Pointwise, Inc.
 * Copyright (c) 1995-2011 Pointwise, Inc.
 * All rights reserved.
 *
 ***************************************************************************/

#include "apiPWP.h"
#include "CaeUnsGridModel.h"
#include "cml.h"
#include "PolyBuilder.h"
#include "Polygon.h"


bool
PolyBuilder::build(const VertElemRange &rng)
{
    bool ret = true;

    // rng.first is iter to range's first valid VertElemMap item. rng.second
    // is iter to the item just AFTER range's last valid VertElemMap item.
    VertElemMap::const_iterator veIt = rng.first;
    VertElemMap::const_iterator veEndIt = rng.second;

    // poly_.owner_ is index common to all elements in rng. It is the "owner"
    // of the resulting recombined polygon.
    poly_.owner_ = veIt->first;

    PWGM_ELEMDATA data;
    PWP_UINT32 elemCnt = (PWP_UINT32)std::distance(veIt, veEndIt);
    poly_.clear(elemCnt);
    PolygonArray unmatchedEdges;
    PolygonArray::iterator polyIt;
    for (; veIt != veEndIt; ++veIt) { // elemId0 ... elemIdN
        CaeUnsElement e(rng.block(), veIt->second);
        if(!e.data(data) || (PWGM_ELEMTYPE_TRI != data.type)) {
            // data failed (bad) or elem not a tri (not combineable)
            ret = false;
            break;
        }
        if (poly_.owner_ == data.index[0]) {
            // no rotate needed! poly_.owner_ is already at index[0]
        }
        else if (poly_.owner_ == data.index[1]) {
            // rotate indices so poly_.owner_ is always at index[0]
            data.index[3] = data.index[0];
            data.index[0] = data.index[1];
            data.index[1] = data.index[2];
            data.index[2] = data.index[3];
        }
        else if (poly_.owner_ == data.index[2]) {
            // rotate indices so poly_.owner_ is always at index[0]
            data.index[3] = data.index[2];
            data.index[2] = data.index[1];
            data.index[1] = data.index[0];
            data.index[0] = data.index[3];
        }
        else {
            ret = false; // should never get here
            break;
        }
        if (ret) {
            // Push radial edges. Used for ply quality calculaton.
            pushRadial(data.index[0], data.index[1]);
            pushRadial(data.index[0], data.index[2]);
            // Add the edge opposite of poly_.owner_ to poly. Always add the edge
            // vertices in a consistent, right or left-handed order. Using
            // right-handed ordering here.
            if (!poly_.addEdge(data.index[1], data.index[2])) {
                // try to add this edge to polys in unmatchedEdges
                bool edgeAdded = false;
                polyIt = unmatchedEdges.begin();
                for (; polyIt != unmatchedEdges.end(); ++polyIt) {
                    if (polyIt->addEdge(data.index[1], data.index[2])) {
                        edgeAdded = true;
                        break;
                    }
                }
                // if not added, this edge is disjoint. Append a new poly to
                // unmatchedEdges that contains this edge
                if (!edgeAdded) {
                    // Add poly constructed with the single disjoint edge.
                    // Reserve same capacity as poly_.
                    unmatchedEdges.push_back(Polygon(data.index[1],
                        data.index[2], elemCnt));
                }
            }
        }
    }
    if (ret && !unmatchedEdges.empty()) {
        // Join polys in unmatchedEdges to the main poly. Keep going until
        // unmatchedEdges is empty or no join found in a pass.
        bool hadJoin = true;
        while (hadJoin && !unmatchedEdges.empty()) {
            hadJoin = false;
            polyIt = unmatchedEdges.begin();
            for (; polyIt != unmatchedEdges.end(); ++polyIt) {
                if (poly_.join(*polyIt)) {
                    // don't need this poly anymore
                    unmatchedEdges.erase(polyIt);
                    hadJoin = true;
                    // polyIt is no longer valid. Let while loop go again
                    break;
                }
            }
        }
        ret = unmatchedEdges.empty();
    }

    // push quality value to the poly
    poly_.quality_ = quality();

    // poly is not closed if if counts don't match
    return ret && (poly_.size() == elemCnt);
}

bool
PolyBuilder::polyIsConvex() const
{
    PWP_UINT32 polySize = poly_.size();
    bool ret = (polySize >= 3);
    if (polySize > 3) { // if size is 3, no need to check. return true
        const UIntArray& indices = poly_.indices();
        UIntArray::const_iterator it = indices.begin();
        // preload last and first 2 verts of poly
        CaeUnsVertex v0(model_, indices.at(polySize - 1));
        CaeUnsVertex v1(model_, *it++);
        CaeUnsVertex v2(model_, *it++); // it will ref next poly index
        // preload xyz pts
        vector3 pt0;
        vector3 pt1;
        vector3 pt2;
        vector3 firstPt;
        ret = vertToVec3(v0, pt0) && vertToVec3(v1, pt1) &&
                vertToVec3(v2, pt2);
        if (ret) {
            firstPt = pt1; // cache for use below
            // preload normalized edges
            vector3 e01 = (pt1 - pt0).normalize();
            vector3 e12 = (pt2 - pt1).normalize();
            // preload normal (of indices[0])
            vector3 n0 = cml::cross(e01, e12);
            // shift everything to prepare for first loop pass
            v0 = v1;
            pt0 = pt1;
            v1 = v2;
            pt1 = pt2;
            e01 = e12;
            vector3 n1;
            for (; it != indices.end(); ++it) {
                // get next vert of poly
                v2.moveTo(model_, *it); // grab next vert of poly
                if (!vertToVec3(v2, pt2)) {
                    ret = false;
                    break;
                }
                // next normalized edge
                e12 = (pt2 - pt1).normalize();
                // next normal
                n1 = cml::cross(e01, e12);
                if (cml::dot(n0, n1) < 0) {
                    ret = false;
                    break;
                }
                // shift everything to prepare for next loop pass
                n0 = n1;
                v0 = v1;
                pt0 = pt1;
                v1 = v2;
                pt1 = pt2;
                e01 = e12;
            }
            if (ret) {
                // wrap around to first vert of poly
                // next normalized edge
                e12 = (firstPt - pt1).normalize();
                // next normal
                n1 = cml::cross(e01, e12);
                if (cml::dot(n0, n1) < 0) {
                    ret = false;
                }
            }
        }
    }
    return ret;
}

PWP_FLOAT
PolyBuilder::quality() const
{
    // Poly quality is the ratio between minRSquared and maxRSquared. This
    // represents a comparision of the circular areas of the min/max radii.
    // A perfect, regular poly (all radii equal) will have a ratio of 1.
    // Don't need pi in this calc because the division cancels it out:
    //
    //   (pi*minRSquared)/(pi*maxRSquared) == minRSquared/maxRSquared
    //
    if (quality_ < 0) {
        // div by zero check
        if (maxRSquared_ < 1e-9) {
            quality_ = 0;
        }
        else {
            quality_ = (minRSquared_ / maxRSquared_);
        }
    }
    return quality_;
}

bool
PolyBuilder::vertToVec3(const CaeUnsVertex &vertex, vector3 &v3)
{
    PWGM_VERTDATA vd;
    bool ret = vertex.dataMod(vd);
    if (ret) {
        v3.set(vd.x, vd.y, vd.z);
    }
    return ret;
}


void
PolyBuilder::pushRadial(PWP_UINT32 i0, PWP_UINT32 i1)
{
    vector3 pt0;
    vector3 pt1;
    if (vertToVec3(CaeUnsVertex(model_, i0), pt0) &&
            vertToVec3(CaeUnsVertex(model_, i1), pt1)) {
        pushRSquared((PWP_FLOAT)(pt1 - pt0).length_squared());
    }
}

void
PolyBuilder::pushRSquared(PWP_FLOAT rSquared)
{
    if (rSquared > 0.0) {
        if (rSquared < minRSquared_) {
            minRSquared_ = rSquared;
            quality_ = -1.0;
        }
        else if (rSquared > maxRSquared_) {
            maxRSquared_ = rSquared;
            quality_ = -1.0;
        }
    }
}
