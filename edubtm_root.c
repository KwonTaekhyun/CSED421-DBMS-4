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
 * Module: edubtm_root.c
 *
 * Description : 
 *  This file has two routines which are concerned with the changing the
 *  current root node. When an overflow or a underflow occurs in the root page
 *  the root node should be changed. But we don't change the root node to
 *  the new page. The old root page is used as the new root node; thus the
 *  root page is fixed always.
 *
 * Exports:
 *  Four edubtm_root_insert(ObjectID*, PageID*, InternalItem*)
 */


#include <string.h>
#include "EduBtM_common.h"
#include "Util.h"
#include "BfM.h"
#include "EduBtM_Internal.h"



/*@================================
 * edubtm_root_insert()
 *================================*/
/*
 * Function: Four edubtm_root_insert(ObjectID*, PageID*, InternalItem*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  This routine is called when a new entry was inserted into the root page and
 *  it was splitted two pages, 'root' and 'item->pid'. The new root should be
 *  made by the given root Page IDentifier and the sibling entry 'item'.
 *  We make it a rule to fix the root page; so a new page is allocated and
 *  the root node is copied into the newly allocated page. The root node
 *  is changed so that it points to the newly allocated node and the 'item->pid'.
 *
 * Returns:
 *  Error code
 *    some errors caused by function calls
 */
Four edubtm_root_insert(
    ObjectID     *catObjForFile, /* IN catalog object of B+ tree file */
    PageID       *root,		 /* IN root Page IDentifier */
    InternalItem *item)		 /* IN Internal item which will be the unique entry of the new root */
{
    Four      e;		/* error number */
    PageID    newPid;		/* newly allocated page */
    PageID    nextPid;		/* PageID of the next page of root if root is leaf */
    BtreePage *rootPage;	/* pointer to a buffer holding the root page */
    BtreePage *newPage;		/* pointer to a buffer holding the new page */
    BtreeLeaf *nextPage;	/* pointer to a buffer holding next page of root */
    btm_InternalEntry *entry;	/* an internal entry */
    Boolean   isTmp;

    // Root page가 split 된 B+ tree 색인을 위한 새로운 root page를 생성함

    // 1) 새로운 page를 할당 받음
    btm_AllocPage(catObjForFile, root, &newPid);
    BfM_GetNewTrain(&newPid, &newPage, PAGE_BUF);
    BfM_GetTrain(root, &rootPage, PAGE_BUF);
    // 2) 기존 root page를 할당 받은 page로 복사함
    memcpy(newPage, rootPage, sizeof(BtreePage));
    // 3) 기존 root page를 새로운 root page로서 초기화함 (B+ tree 색인의 root page의 page ID를 일관되게 유지하기 위함)
    edubtm_InitInternal(root, TRUE, FALSE);

    newPage->any.hdr.pid=newPid;
    newPage->bi.hdr.pid=newPid;
    newPage->bl.hdr.pid=newPid;

    // 4) 할당 받은 page와 root page split으로 생성된 page가 새로운 root page의 자식 page들이 되도록 설정함
    
    // 4-1) Split으로 생성된 page를 가리키는 internal index entry를 새로운 root page에 삽입함
    entry = rootPage->bi.data + rootPage->bi.hdr.free;
	entry->spid = item->spid;
	entry->klen = item->klen;
	memcpy(entry->kval, item->kval, entry->klen);

    rootPage->bi.slot[-1*rootPage->bi.hdr.nSlots] = rootPage->bi.hdr.free;
	rootPage->bi.hdr.free += sizeof(ShortPageID) + 2*sizeof(Two) + ALIGNED_LENGTH(entry->klen);
	rootPage->bi.hdr.nSlots++;

    // 4-2) 새로운 root page의 header의 p0 변수에 할당 받은 page의 번호를 저장함
    rootPage->bi.hdr.p0 = newPid.pageNo;

    // 4-3) 새로운 root page의 두 자식 page들이 leaf인 경우, 두 자식 page들간의 doubly linked list를 설정함
    //     (Split으로 생성된 page가 할당 받은 page의 다음 page가 되도록 설정함)
    MAKE_PAGEID(nextPid, root->volNo, entry->spid);
    BfM_GetTrain(&nextPid, &nextPage, PAGE_BUF);

    if ((newPage->any.hdr.type & LEAF) && (nextPage->hdr.type & LEAF))
	{
		newPage->bl.hdr.nextPage = nextPid.pageNo;
		nextPage->hdr.prevPage = newPid.pageNo;	
	}

    BfM_SetDirty(&nextPid, PAGE_BUF);
    BfM_SetDirty(&newPid, PAGE_BUF);
    BfM_SetDirty(root, PAGE_BUF);

    BfM_FreeTrain(&nextPid, PAGE_BUF);
    BfM_FreeTrain(&newPid, PAGE_BUF);
    BfM_FreeTrain(root, PAGE_BUF);
    
    return(eNOERROR);
    
} /* edubtm_root_insert() */
