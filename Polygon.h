/****************************************************************************
 *
 * class Polygon
 *
 * Proprietary software product of Pointwise, Inc.
 * Copyright (c) 1995-2013 Pointwise, Inc.
 * All rights reserved.
 *
 ***************************************************************************/

#ifndef _POLYGON_H_
#define _POLYGON_H_

#include <cstring>
#include <vector>
#include "apiPWP.h"

class PolyBuilder;

typedef std::vector<PWP_UINT32> UIntArray;


//***************************************************************************
//***************************************************************************
//***************************************************************************

class Polygon {

    friend class PolyBuilder;

public:
    Polygon(UIntArray::size_type count = 0) :
        owner_(PWP_UINT32_MAX),
        quality_(0)
    {
        indices_.reserve(count);
    }

    Polygon(PWP_UINT32 e0, PWP_UINT32 e1, UIntArray::size_type count = 0) :
        owner_(PWP_UINT32_MAX),
        quality_(0)
    {
        indices_.reserve((count < 2) ? 2 : count);
        indices_.push_back(e0);
        indices_.push_back(e1);
    }

    Polygon(const PWP_UINT32 *indices, UIntArray::size_type count) :
        owner_(PWP_UINT32_MAX),
        quality_(0)
    {
        indices_.reserve(count);
        indices_.insert(indices_.end(), indices, indices + count);
    }

    ~Polygon(){
    }

    UIntArray& indices() {
        return indices_;
    }

    const UIntArray& indices() const {
        return indices_;
    }

    PWP_FLOAT quality() const {
        return quality_;
    }

    PWP_UINT32 owner() const {
        return owner_;
    }

    PWP_UINT32 isUnowned() const {
        return PWP_UINT32_MAX == owner_;
    }

    void clear(UIntArray::size_type count = 0) {
        indices_.clear(); // destroy existing indices
        indices_.reserve(count); // make sure enough capacity
    }

    bool join(const Polygon &other);

    bool addEdge(PWP_UINT32 e0, PWP_UINT32 e1);

    PWP_UINT32 size() const {
        return (PWP_UINT32)indices_.size();
    }

    const char * toString(const char *delim = 0, char *buf = 0) const
    {
        if (0 == buf) {
            // buf contents only valid until next call
            static char singleUseBuf[512];
            buf = singleUseBuf;
        }
        if (0 == delim) {
            delim = " ";
        }
        char * p = buf;
        UIntArray::const_iterator itNdx = indices_.begin();
        for (; itNdx != indices_.end(); ++itNdx) {
            p += sprintf(p, "%lu%s", (unsigned long)*itNdx, delim);
        }
        if (0 != *delim) {
            p -= strlen(delim);
            *p = '\0'; // strip trailing delim
        }
        return buf; // skip leading space
    }

private:

    UIntArray   indices_;
    PWP_UINT32  owner_;
    PWP_FLOAT   quality_;
};


//***************************************************************************
//***************************************************************************
//***************************************************************************

typedef std::vector<Polygon>    PolygonArray;

#endif // _POLYGON_H_
