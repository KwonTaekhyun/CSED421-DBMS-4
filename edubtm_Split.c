/******************************************************************************/
/*                                                                            */
/*    ODYSSEUS/EduCOSMOS Educational-Purpose Object Storage System            */
/*                                                                            */
/*    Developed by Professor Kyu-Young Whang et al.                           */
/*                                                                            */
/*    Database and Multimedia Laboratory                                      */
/*                                                                            */
/*    Computer Science Department and                                         */
/*    Advanced Information Technology Research Center (AITrc)                 */
/*    Korea Advanced Institute of Science and Technology (KAIST)              */
/*                                                                            */
/*    e-mail: kywhang@cs.kaist.ac.kr                                          */
/*    phone: +82-42-350-7722                                                  */
/*    fax: +82-42-350-8380                                                    */
/*                                                                            */
/*    Copyright (c) 1995-2013 by Kyu-Young Whang                              */
/*                                                                            */
/*    All rights reserved. No part of this software may be reproduced,        */
/*    stored in a retrieval system, or transmitted, in any form or by any     */
/*    means, electronic, mechanical, photocopying, recording, or otherwise,   */
/*    without prior written permission of the copyright owner.                */
/*                                                                            */
/******************************************************************************/
/*
 * Module: edubtm_Split.c
 *
 * Description : 
 *  This file has three functions about 'split'.
 *  'edubtm_SplitInternal(...) and edubtm_SplitLeaf(...) insert the given item
 *  after spliting, and return 'ritem' which should be inserted into the
 *  parent page.
 *
 * Exports:
 *  Four edubtm_SplitInternal(ObjectID*, BtreeInternal*, Two, InternalItem*, InternalItem*)
 *  Four edubtm_SplitLeaf(ObjectID*, PageID*, BtreeLeaf*, Two, LeafItem*, InternalItem*)
 */


#include <string.h>
#include "EduBtM_common.h"
#include "BfM.h"
#include "EduBtM_Internal.h"



/*@================================
 * edubtm_SplitInternal()
 *================================*/
/*
 * Function: Four edubtm_SplitInternal(ObjectID*, BtreeInternal*,Two, InternalItem*, InternalItem*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  At first, the function edubtm_SplitInternal(...) allocates a new internal page
 *  and initialize it.  Secondly, all items in the given page and the given
 *  'item' are divided by halves and stored to the two pages.  By spliting,
 *  the new internal item should be inserted into their parent and the item will
 *  be returned by 'ritem'.
 *
 *  A temporary page is used because it is difficult to use the given page
 *  directly and the temporary page will be copied to the given page later.
 *
 * Returns:
 *  error code
 *    some errors caused by function calls
 *
 * Note:
 *  The caller should call BfM_SetDirty() for 'fpage'.
 */
Four edubtm_SplitInternal(
    ObjectID                    *catObjForFile,         /* IN catalog object of B+ tree file */
    BtreeInternal               *fpage,                 /* INOUT the page which will be splitted */
    Two                         high,                   /* IN slot No. for the given 'item' */
    InternalItem                *item,                  /* IN the item which will be inserted */
    InternalItem                *ritem)                 /* OUT the item which will be returned by spliting */
{
    Four                        e;                      /* error number */
    Two                         i;                      /* slot No. in the given page, fpage */
    Two                         j;                      /* slot No. in the splitted pages */
    Two                         k;                      /* slot No. in the new page */
    Two                         maxLoop;                /* # of max loops; # of slots in fpage + 1 */
    Four                        sum;                    /* the size of a filled area */
    Boolean                     flag=FALSE;             /* TRUE if 'item' become a member of fpage */
    PageID                      newPid;                 /* for a New Allocated Page */
    BtreeInternal               *npage;                 /* a page pointer for the new allocated page */
    Two                         fEntryOffset;           /* starting offset of an entry in fpage */
    Two                         nEntryOffset;           /* starting offset of an entry in npage */
    Two                         entryLen;               /* length of an entry */
    btm_InternalEntry           *fEntry;                /* internal entry in the given page, fpage */
    btm_InternalEntry           *nEntry;                /* internal entry in the new page, npage*/
    Boolean                     isTmp;


    
    return(eNOERROR);
    
} /* edubtm_SplitInternal() */



/*@================================
 * edubtm_SplitLeaf()
 *================================*/
/*
 * Function: Four edubtm_SplitLeaf(ObjectID*, PageID*, BtreeLeaf*, Two, LeafItem*, InternalItem*)
 *
 * Description: 
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  The function edubtm_SplitLeaf(...) is similar to edubtm_SplitInternal(...) except
 *  that the entry of a leaf differs from the entry of an internal and the first
 *  key value of a new page is used to make an internal item of their parent.
 *  Internal pages do not maintain the linked list, but leaves do it, so links
 *  are properly updated.
 *
 * Returns:
 *  Error code
 *  eDUPLICATEDOBJECTID_BTM
 *    some errors caused by function calls
 *
 * Note:
 *  The caller should call BfM_SetDirty() for 'fpage'.
 */
Four edubtm_SplitLeaf(
    ObjectID                    *catObjForFile, /* IN catalog object of B+ tree file */
    PageID                      *root,          /* IN PageID for the given page, 'fpage' */
    BtreeLeaf                   *fpage,         /* INOUT the page which will be splitted */
    Two                         high,           /* IN slotNo for the given 'item' */
    LeafItem                    *item,          /* IN the item which will be inserted */
    InternalItem                *ritem)         /* OUT the item which will be returned by spliting */
{
    Four                        e;              /* error number */
    Two                         i;              /* slot No. in the given page, fpage */
    Two                         j;              /* slot No. in the splitted pages */
    Two                         k;              /* slot No. in the new page */
    Two                         maxLoop;        /* # of max loops; # of slots in fpage + 1 */
    Four                        sum;            /* the size of a filled area */
    PageID                      newPid;         /* for a New Allocated Page */
    PageID                      nextPid;        /* for maintaining doubly linked list */
    BtreeLeaf                   tpage;          /* a temporary page for the given page */
    BtreeLeaf                   *npage;         /* a page pointer for the new page */
    BtreeLeaf                   *mpage;         /* for doubly linked list */
    btm_LeafEntry               *itemEntry;     /* entry for the given 'item' */
    btm_LeafEntry               *fEntry;        /* an entry in the given page, 'fpage' */
    btm_LeafEntry               *nEntry;        /* an entry in the new page, 'npage' */
    ObjectID                    *iOidArray;     /* ObjectID array of 'itemEntry' */
    ObjectID                    *fOidArray;     /* ObjectID array of 'fEntry' */
    Two                         fEntryOffset;   /* starting offset of 'fEntry' */
    Two                         nEntryOffset;   /* starting offset of 'nEntry' */
    Two                         oidArrayNo;     /* element No in an ObjectID array */
    Two                         alignedKlen;    /* aligned length of the key length */
    Two                         itemEntryLen;   /* length of entry for item */
    Two                         entryLen;       /* entry length */
    Boolean                     flag;
    Boolean                     isTmp;
 
    // Overflow가 발생한 leaf page를 split 하여 파라미터로 주어진 index entry를 삽입하고, split으로 생성된 새로운 leaf page를 가리키는 internal index entry를 반환함

    // 1) 새로운 page를 할당 받음
    btm_AllocPage(catObjForFile, &fpage->hdr.pid, &newPid);
    // 2) 할당 받은 page를 leaf page로 초기화함
    edubtm_InitLeaf(&newPid, FALSE, isTmp);
    // 3) 기존 index entry들 및 삽입할 index entry를 key 순으로 정렬하여 overflow가 발생한 page 및 할당 받은 page에 나누어 저장함
    BfM_GetNewTrain(&newPid, (char**)&npage, PAGE_BUF);
    // 먼저, overflow가 발생한 page에 데이터 영역을 50% 이상 채우는 수의 index entry들을 저장하고 header 갱신
    sum = 0;
    maxLoop = fpage->hdr.nSlots;
    for (i = 0; i < maxLoop && sum < BL_HALF; i++) {
        fEntryOffset = fpage->slot[-i];
        fEntry = (btm_LeafEntry*)&(fpage->data[fEntryOffset]);
        alignedKlen = ALIGNED_LENGTH(fEntry->klen);
        sum += sizeof(Two) + sizeof(Two) + alignedKlen + fEntry->nObjects * OBJECTID_SIZE;

        if (i == high) {
            alignedKlen = ALIGNED_LENGTH(fEntry->klen);
            entryLen = sizeof(Two) + sizeof(Two) + alignedKlen + OBJECTID_SIZE;
            sum += entryLen;
        }
    }
    fpage->hdr.nSlots = i;
    // 나머지 index entry들을 할당 받은 page에 저장하고 header 갱신
    flag = FALSE;
    nEntryOffset = 0;
    for (j = 0; i < maxLoop; i++, j++) {
        npage->slot[-j] = nEntryOffset;
        fEntry = (btm_LeafEntry*)&fpage->data[fpage->slot[-i]];
        alignedKlen = ALIGNED_LENGTH(fEntry->klen);
        entryLen = sizeof(Two) + sizeof(Two) + alignedKlen + fEntry->nObjects * OBJECTID_SIZE;
        memcpy(&npage->data[nEntryOffset], fEntry, entryLen);
        nEntryOffset += entryLen;
        npage->hdr.nSlots++;
        fpage->hdr.unused += entryLen;

        if (i == high) {
            j++;
            npage->slot[-j] = nEntryOffset;
            alignedKlen = ALIGNED_LENGTH(item->klen);
            entryLen = sizeof(Two) + sizeof(Two) + alignedKlen + OBJECTID_SIZE;
            memcpy(&npage->data[nEntryOffset], &item->nObjects, entryLen - OBJECTID_SIZE);
            nEntryOffset += entryLen - OBJECTID_SIZE;
            memcpy(&npage->data[nEntryOffset], &item->oid, OBJECTID_SIZE);
            nEntryOffset += OBJECTID_SIZE;
            npage->hdr.nSlots++;
            flag = TRUE;
        }
    }
    npage->hdr.free = nEntryOffset;
    edubtm_CompactLeafPage(fpage, flag ? NIL : high);
    if (!flag) {
        for (k = fpage->hdr.nSlots; k > high + 1; k--)
            fpage->slot[-k] = fpage->slot[-k+1];
        fpage->slot[-k] = fpage->hdr.free;
        alignedKlen = ALIGNED_LENGTH(item->klen);
        entryLen = sizeof(Two) + sizeof(Two) + alignedKlen + OBJECTID_SIZE;
        fEntryOffset = fpage->hdr.free;
        memcpy(&fpage->data[fEntryOffset], &item->nObjects, entryLen - OBJECTID_SIZE);
        fEntryOffset += entryLen - OBJECTID_SIZE;
        memcpy(&fpage->data[fEntryOffset], &item->oid, OBJECTID_SIZE);
        fpage->hdr.free += entryLen;
        fpage->hdr.nSlots++;
    }
    // 4) 할당 받은 page를 leaf page들간의 doubly linked list에 추가함
    npage->hdr.nextPage = fpage->hdr.nextPage;
    npage->hdr.prevPage = fpage->hdr.pid.pageNo;
    fpage->hdr.nextPage = newPid.pageNo;
    // 5) 할당 받은 page를 가리키는 internal index entry를 생성함
    // 6) Split된 page가 ROOT일 경우, type을 LEAF로 변경함
    // 7) 생성된 index entry를 반환함
    BfM_SetDirty(&newPid, PAGE_BUF);
    BfM_SetDirty(&fpage->hdr.pid, PAGE_BUF);
    BfM_FreeTrain(&newPid, PAGE_BUF);

    return(eNOERROR);
    
} /* edubtm_SplitLeaf() */
