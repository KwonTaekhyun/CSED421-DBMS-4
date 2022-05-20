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
 * Module: edubtm_Insert.c
 *
 * Description : 
 *  This function edubtm_Insert(...) recursively calls itself until the type
 *  of a root page becomes LEAF.  If the given root page is an internal,
 *  it recursively calls itself using a proper child.  If the result of
 *  the call occur spliting, merging, or redistributing the children, it
 *  may insert, delete, or replace its own internal item, and if the given
 *  root page may be merged, splitted, or redistributed, it affects the
 *  return values.
 *
 * Exports:
 *  Four edubtm_Insert(ObjectID*, PageID*, KeyDesc*, KeyValue*, ObjectID*,
 *                  Boolean*, Boolean*, InternalItem*, Pool*, DeallocListElem*)
 *  Four edubtm_InsertLeaf(ObjectID*, PageID*, BtreeLeaf*, KeyDesc*, KeyValue*,
 *                      ObjectID*, Boolean*, Boolean*, InternalItem*)
 *  Four edubtm_InsertInternal(ObjectID*, BtreeInternal*, InternalItem*,
 *                          Two, Boolean*, InternalItem*)
 */


#include <string.h>
#include "EduBtM_common.h"
#include "BfM.h"
#include "OM_Internal.h"	/* for SlottedPage containing catalog object */
#include "EduBtM_Internal.h"



/*@================================
 * edubtm_Insert()
 *================================*/
/*
 * Function: Four edubtm_Insert(ObjectID*, PageID*, KeyDesc*, KeyValue*,
 *                           ObjectID*, Boolean*, Boolean*, InternalItem*,
 *                           Pool*, DeallocListElem*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  If the given root is a leaf page, it should get the correct entry in the
 *  leaf. If the entry is already in the leaf, it simply insert it into the
 *  entry and increment the number of ObjectIDs.  If it is not in the leaf it
 *  makes a new entry and insert it into the leaf.
 *  If there is not enough spage in the leaf, the page should be splitted.  The
 *  overflow page may be used or created by this routine. It is created when
 *  the size of the entry is greater than a third of a page.
 * 
 *  'h' is TRUE if the given root page is splitted and the entry item will be
 *  inserted into the parent page.  'f' is TRUE if the given page is not half
 *  full because of creating a new overflow page.
 *
 * Returns:
 *  Error code
 *    eBADBTREEPAGE_BTM
 *    some errors caused by function calls
 */
Four edubtm_Insert(
    ObjectID                    *catObjForFile,         /* IN catalog object of B+-tree file */
    PageID                      *root,                  /* IN the root of a Btree */
    KeyDesc                     *kdesc,                 /* IN Btree key descriptor */
    KeyValue                    *kval,                  /* IN key value */
    ObjectID                    *oid,                   /* IN ObjectID which will be inserted */
    Boolean                     *f,                     /* OUT whether it is merged by creating a new overflow page */
    Boolean                     *h,                     /* OUT whether it is splitted */
    InternalItem                *item,                  /* OUT Internal Item which will be inserted */
                                                        /*     into its parent when 'h' is TRUE */
    Pool                        *dlPool,                /* INOUT pool of dealloc list */
    DeallocListElem             *dlHead)                /* INOUT head of the dealloc list */
{
    Four                        e;                      /* error number */
    Boolean                     lh;                     /* local 'h' */
    Boolean                     lf;                     /* local 'f' */
    Two                         idx;                    /* index for the given key value */
    PageID                      newPid;                 /* a new PageID */
    KeyValue                    tKey;                   /* a temporary key */
    InternalItem                litem;                  /* a local internal item */
    BtreePage                   *apage;                 /* a pointer to the root page */
    btm_InternalEntry           *iEntry;                /* an internal entry */
    Two                         iEntryOffset;           /* starting offset of an internal entry */
    SlottedPage                 *catPage;               /* buffer page containing the catalog object */
    sm_CatOverlayForBtree       *catEntry;              /* pointer to Btree file catalog information */
    PhysicalFileID              pFid;                   /* B+-tree file's FileID */


    /* Error check whether using not supported functionality by EduBtM */
    int i;
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }
    
    // 파라미터로 주어진 page를 root page로 하는 B+ tree 색인에 새로운 object에 대한 <object의 key, object ID> pair를 삽입하고, root page에서 split이 발생한 경우, split으로 생성된 새로운 page를 가리키는 internal index entry를 반환함

    *h=FALSE;
    *f=FALSE;

    e = BfM_GetTrain(root, &apage, PAGE_BUF);
	if(e) ERR(e);

    if (apage->any.hdr.type & INTERNAL) {
        // 1) 파라미터로 주어진 root page가 internal page인 경우
        // 1-1) 새로운 <object의 key, object ID> pair를 삽입할 leaf page를 찾기 위해 다음으로 방문할 자식 page를 결정함
        edubtm_BinarySearchInternal(apage, kdesc, kval, &idx);
        // 1-2) 결정된 자식 page를 root page로 하는 B+ subtree에 새로운 <object의 key, object ID> pair를 삽입하기 위해 재귀적으로 edubtm_Insert()를 호출함

        iEntryOffset = apage->bi.slot[-idx];
        iEntry = (btm_InternalEntry*) &apage->bi.data[iEntryOffset];

        if(idx >= 0){
            MAKE_PAGEID(newPid, root->volNo, iEntry->spid);
        }
        else{
            MAKE_PAGEID(newPid, root->volNo, apage->bi.hdr.p0);
        }

        lh=FALSE;
        lf=FALSE;
        
        edubtm_Insert(catObjForFile, &newPid, kdesc, kval, oid, &lf, &lh, &litem, dlPool, dlHead);
        // 1-3) 결정된 자식 page에서 split이 발생한 경우, 해당 split으로 생성된 새로운 page를 가리키는 internal index entry를 파라미터로 주어진 root page에 삽입함
        // 1-4) 파라미터로 주어진 root page에서 split이 발생한 경우, 해당 split으로 생성된 새로운 page를 가리키는 internal index entry를 반환함
        if (lh) {
            tKey.len= litem.klen;
            memcpy(tKey.val, litem.kval, tKey.len);
            edubtm_BinarySearchInternal(&(apage->bi), kdesc, &tKey, &idx);
            edubtm_InsertInternal(catObjForFile, &(apage->bi), &litem, idx, h, item);
        }
    }
    else if (apage->any.hdr.type & LEAF) {
        // 2) 파라미터로 주어진 root page가 leaf page인 경우
        // 2-1) edubtm_InsertLeaf()를 호출하여 해당 page에 새로운 <object의 key, object ID> pair를 삽입함
        // 2-2) Split이 발생한 경우, 해당 split으로 생성된 새로운 page를 가리키는 internal index entry를 반환함
        edubtm_InsertLeaf(catObjForFile, root, &(apage->bl), kdesc, kval, oid, f, h, item);
    }

    BfM_SetDirty(root, PAGE_BUF);
    BfM_FreeTrain(root, PAGE_BUF);
    
    return(eNOERROR);
    
}   /* edubtm_Insert() */



/*@================================
 * edubtm_InsertLeaf()
 *================================*/
/*
 * Function: Four edubtm_InsertLeaf(ObjectID*, PageID*, BtreeLeaf*, KeyDesc*,
 *                               KeyValue*, ObjectID*, Boolean*, Boolean*,
 *                               InternalItem*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Insert into the given leaf page an ObjectID with the given key.
 *
 * Returns:
 *  Error code
 *    eDUPLICATEDKEY_BTM
 *    eDUPLICATEDOBJECTID_BTM
 *    some errors causd by function calls
 *
 * Side effects:
 *  1) f : TRUE if the leaf page is underflowed by creating an overflow page
 *  2) h : TRUE if the leaf page is splitted by inserting the given ObjectID
 *  3) item : item to be inserted into the parent
 */
Four edubtm_InsertLeaf(
    ObjectID                    *catObjForFile, /* IN catalog object of B+-tree file */
    PageID                      *pid,           /* IN PageID of Leag Page */
    BtreeLeaf                   *page,          /* INOUT pointer to buffer page of Leaf page */
    KeyDesc                     *kdesc,         /* IN Btree key descriptor */
    KeyValue                    *kval,          /* IN key value */
    ObjectID                    *oid,           /* IN ObjectID which will be inserted */
    Boolean                     *f,             /* OUT whether it is merged by creating */
                                                /*     a new overflow page */
    Boolean                     *h,             /* OUT whether it is splitted */
    InternalItem                *item)          /* OUT Internal Item which will be inserted */
                                                /*     into its parent when 'h' is TRUE */
{
    Four                        e;              /* error number */
    Two                         i;
    Two                         idx;            /* index for the given key value */
    LeafItem                    leaf;           /* a Leaf Item */
    Boolean                     found;          /* search result */
    btm_LeafEntry               *entry;         /* an entry in a leaf page */
    Two                         entryOffset;    /* start position of an entry */
    Two                         alignedKlen;    /* aligned length of the key length */
    PageID                      ovPid;          /* PageID of an overflow page */
    Two                         entryLen;       /* length of an entry */
    ObjectID                    *oidArray;      /* an array of ObjectIDs */
    Two                         oidArrayElemNo; /* an index for the ObjectID array */


    /* Error check whether using not supported functionality by EduBtM */
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    
    /*@ Initially the flags are FALSE */
    *h = *f = FALSE;

    // Leaf page에 새로운 index entry를 삽입하고, split이 발생한 경우, split으로 생성된 새로운 leaf page를 가리키는 internal index entry를 반환함
	
    // 1) 새로운 index entry의 삽입 위치 (slot 번호) 를 결정함
    found = edubtm_BinarySearchLeaf(page, kdesc, kval, &idx);
    if (found)  ERR(eDUPLICATEDKEY_BTM);

    // 2) 새로운 index entry 삽입을 위해 필요한 자유 영역의 크기를 계산함
	if (kdesc->kpart[0].type == SM_INT)
		alignedKlen = ALIGNED_LENGTH(kdesc->kpart[0].length);
	else
        alignedKlen = ALIGNED_LENGTH(kval->len);
	entryLen = sizeof(Two) * 2 + alignedKlen + OBJECTID_SIZE;

	if (BL_FREE(page) >= entryLen + sizeof(Two)) {
        // 3) Page에 여유 영역이 있는 경우,
        // 3-1) 필요시 page를 compact 함
        if (BL_CFREE(page) < entryLen + sizeof(Two)){
            edubtm_CompactLeafPage(page, NIL);
        }
        // 3-2) 결정된 slot 번호로 새로운 index entry를 삽입함
        // Page의 contiguous free area에 새로운 index entry를 복사함
        // 결정된 slot 번호를 갖는 slot을 사용하기 위해 slot array를 재배열함
        // 결정된 slot 번호를 갖는 slot에 새로운 index entry의 offset을 저장함
        // Page의 header를 갱신함

        leaf.oid = *oid;
        leaf.nObjects = 1;
        leaf.klen = kval->len;
        memcpy(leaf.kval, kval->val, leaf.klen);

        for(i = page->hdr.nSlots; i > (idx+1); i--)
            page->slot[-i] = page->slot[-i+1];

        entryOffset = page->slot[-(idx + 1)] = page->hdr.free;
        entry = (btm_LeafEntry*)&(page->data[entryOffset]);

        memcpy(entry, &leaf.nObjects, entryLen - OBJECTID_SIZE);
        memcpy(&entry->kval[alignedKlen], &leaf.oid, OBJECTID_SIZE);


        page->hdr.free += entryLen;
        page->hdr.nSlots++;
    }
    else{
        // 4) Page에 여유 영역이 없는 경우 (page overflow),
        // 4-1) edubtm_SplitLeaf()를 호출하여 page를 split 함
        // 4-2) Split으로 생성된 새로운 leaf page를 가리키는 internal index entry를 반환함
        leaf.oid = *oid;
        leaf.nObjects = 1;
        leaf.klen = kval->len;
        memcpy(leaf.kval, kval->val, leaf.klen);

        edubtm_SplitLeaf(catObjForFile, pid, page, idx, &leaf, item);

        *h = TRUE;
    }
    return(eNOERROR);
    
} /* edubtm_InsertLeaf() */



/*@================================
 * edubtm_InsertInternal()
 *================================*/
/*
 * Function: Four edubtm_InsertInternal(ObjectID*, BtreeInternal*, InternalItem*, Two, Boolean*, InternalItem*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  This routine insert the given internal item into the given page. If there
 *  is not enough space in the page, it should split the page and the new
 *  internal item should be returned for inserting into the parent.
 *
 * Returns:
 *  Error code
 *    some errors caused by function calls
 *
 * Side effects:
 *  h:	TRUE if the page is splitted
 *  ritem: an internal item which will be inserted into parent
 *          if spliting occurs.
 */
Four edubtm_InsertInternal(
    ObjectID            *catObjForFile, /* IN catalog object of B+-tree file */
    BtreeInternal       *page,          /* INOUT Page Pointer */
    InternalItem        *item,          /* IN Iternal item which is inserted */
    Two                 high,           /* IN index in the given page */
    Boolean             *h,             /* OUT whether the given page is splitted */
    InternalItem        *ritem)         /* OUT if the given page is splitted, the internal item may be returned by 'ritem'. */
{
    Four                e;              /* error number */
    Two                 i;              /* index */
    Two                 entryOffset;    /* starting offset of an internal entry */
    Two                 entryLen;       /* length of the new entry */
    btm_InternalEntry   *entry;         /* an internal entry of an internal page */


    
    /*@ Initially the flag are FALSE */
    *h = FALSE;

    // Internal page에 새로운 index entry를 삽입하고, split이 발생한 경우, split으로 생성된 새로운 internal page를 가리키는 internal index entry를 반환함

    // 1) 새로운 index entry 삽입을 위해 필요한 자유 영역의 크기를 계산함
    entryLen = sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(item->klen);
    
	if (BI_FREE(page) >= entryLen + sizeof(Two))
	{
        // 2) Page에 여유 영역이 있는 경우,
        // 2-1) 필요시 page를 compact 함
        // 2-2) 파라미터로 주어진 slot 번호의 다음 slot 번호로 새로운 index entry를 삽입함
		if (BI_CFREE(page) < entryLen + sizeof(Two))
			edubtm_CompactInternalPage(page, NIL);
		
		for (i = page->hdr.nSlots-1; i >= (high+1); i--)
			page->slot[-1*(i+1)] = page->slot[-1*i];
        page->slot[-(high+1)] = page->hdr.free;

        entry = page->data + page->hdr.free;
		entry->spid = item->spid;
		entry->klen = item->klen;
		memcpy(&entry->kval, item->kval, item->klen);
		
		page->hdr.free += entryLen;
		page->hdr.nSlots++;
	}
	else
	{
        // 3) Page에 여유 영역이 없는 경우 (page overflow),
        // 3-1) edubtm_SplitInternal()을 호출하여 page를 split 함
        // 3-2) Split으로 생성된 새로운 internal page를 가리키는 internal index entry를 반환함
		edubtm_SplitInternal(catObjForFile, page, high, item, ritem);

        *h = TRUE;
	}

    return(eNOERROR);
    
} /* edubtm_InsertInternal() */
