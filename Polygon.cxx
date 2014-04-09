/****************************************************************************
 *
 * class Polygon
 *
 * Proprietary software product of Pointwise, Inc.
 * Copyright (c) 1995-2013 Pointwise, Inc.
 * All rights reserved.
 *
 ***************************************************************************/

#include "apiPWP.h"
#include "Polygon.h"


bool
Polygon::join(const Polygon &other)
{
    bool joined = false;
    if (indices_.empty()) {
        // this poly empty, just copy vertices from other
        indices_ = other.indices_;
        joined = true;
    }
    else {
        // indices_.size() should never be 1. Logic should work anyways. Edges
        // are added such that the unmatched poly vertices are always the first
        // and last values in indices_. Edge(otherHead,otherTail) must match
        // one of thisHead or thisTail to get chained in. By convention, edge
        // direction goes from head --> tail.
        PWP_UINT32 thisHead = indices_.front();
        PWP_UINT32 thisTail = indices_.back();
        PWP_UINT32 otherHead = other.indices_.front();
        PWP_UINT32 otherTail = other.indices_.back();
        if (otherTail == thisHead) {
            if (otherHead != thisTail) {
                // other poly does NOT close *this poly. Prepend all indices
                // except for otherTail. otherHead becomes the new unmatched
                // head index.
                indices_.insert(indices_.begin(), other.indices_.begin(),
                    other.indices_.end() - 1);
                joined = true;
            }
            else {
                // Edge(otherHead,otherTail) is the final closing edge of the
                // polygon! Append all indices except for otherHead and
                // otherTail. Appending here to prevent memory shifting.
                indices_.insert(indices_.end(), other.indices_.begin() + 1,
                    other.indices_.end() - 1);
                joined = true;
            }
        }
        else if (thisTail == otherHead) {
            // Insert otherTail as the new unmatched tail index.
            if (thisHead != otherTail) {
                // other poly does NOT close *this poly. Append all indices
                // except for otherHead. otherTail becomes the new unmatched
                // tail index.
                indices_.insert(indices_.end(), other.indices_.begin() + 1,
                    other.indices_.end());
                joined = true;
            }
            else {
                // If here, something unexpected happened. This case should
                // have been handled by if(otherTail == thisHead) test above.
                joined = true;
            }
        }
        // else can't join! other poly is disjoint from *this poly!
    }
    return joined;
}

bool
Polygon::addEdge(PWP_UINT32 e0, PWP_UINT32 e1) {
    return join(Polygon(e0, e1));
}
