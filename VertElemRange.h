/****************************************************************************
 *
 * class CaeUnsRecomb2DPoly
 *
 * Proprietary software product of Pointwise, Inc.
 * Copyright (c) 1995-2013 Pointwise, Inc.
 * All rights reserved.
 *
 ***************************************************************************/

#ifndef _VERTELEMRANGE_H_
#define _VERTELEMRANGE_H_

#include <unordered_map>

class CaeUnsBlock;

typedef STDC11::unordered_multimap<PWP_UINT32, PWP_UINT32> UIntUIntMultiMap;

typedef UIntUIntMultiMap        VertElemMap;
typedef VertElemMap::value_type VertElemVal;

typedef std::pair<VertElemMap::const_iterator, VertElemMap::const_iterator>
                                VertElemRangePair;


//***************************************************************************
//***************************************************************************
//***************************************************************************

class VertElemRange : public VertElemRangePair {
public:
    VertElemRange(const CaeUnsBlock & blk) :
        blk_(blk)
    {
    }

    ~VertElemRange() {
    }

    void set(const VertElemRangePair &rng) {
        *((VertElemRangePair*)this) = rng;
    }

    bool isEmpty() const {
        return first == second;
    }

    const CaeUnsBlock & block() const {
        return blk_;
    }

private:

    const CaeUnsBlock & blk_;
};

#endif // _VERTELEMRANGE_H_
