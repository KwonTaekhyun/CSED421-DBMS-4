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
 * Module: EduBtM_Fetch.c
 *
 * Description :
 *  Find the first object satisfying the given condition.
 *  If there is no such object, then return with 'flag' field of cursor set
 *  to CURSOR_EOS. If there is an object satisfying the condition, then cursor
 *  points to the object position in the B+ tree and the object identifier
 *  is returned via 'cursor' parameter.
 *  The condition is given with a key value and a comparison operator;
 *  the comparison operator is one among SM_BOF, SM_EOF, SM_EQ, SM_LT, SM_LE, SM_GT, SM_GE.
 *
 * Exports:
 *  Four EduBtM_Fetch(PageID*, KeyDesc*, KeyValue*, Four, KeyValue*, Four, BtreeCursor*)
 */


#include <string.h>
#include "EduBtM_common.h"
#include "BfM.h"
#include "EduBtM_Internal.h"


/*@ Internal Function Prototypes */
Four edubtm_Fetch(PageID*, KeyDesc*, KeyValue*, Four, KeyValue*, Four, BtreeCursor*);



/*@================================
 * EduBtM_Fetch()
 *================================*/
/*
 * Function: Four EduBtM_Fetch(PageID*, KeyDesc*, KeyVlaue*, Four, KeyValue*, Four, BtreeCursor*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Find the first object satisfying the given condition. See above for detail.
 *
 * Returns:
 *  error code
 *    eBADPARAMETER_BTM
 *    some errors caused by function calls
 *
 * Side effects:
 *  cursor  : The found ObjectID and its position in the Btree Leaf
 *            (it may indicate a ObjectID in an  overflow page).
 */
Four EduBtM_Fetch(
    PageID   *root,		/* IN The current root of the subtree */
    KeyDesc  *kdesc,		/* IN Btree key descriptor */
    KeyValue *startKval,	/* IN key value of start condition */
    Four     startCompOp,	/* IN comparison operator of start condition */
    KeyValue *stopKval,		/* IN key value of stop condition */
    Four     stopCompOp,	/* IN comparison operator of stop condition */
    BtreeCursor *cursor)	/* OUT Btree Cursor */
{
    int i;
    Four e;		   /* error number */

    
    if (root == NULL) ERR(eBADPARAMETER_BTM);

    /* Error check whether using not supported functionality by EduBtM */
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    // B+ tree 색인에서 검색 조건을 만족하는 첫 번째 object를 검색하고, 검색된 object를 가리키는 cursor를 반환함

    if (startCompOp == SM_BOF){
        // 1) 파라미터로 주어진 startCompOp가 SM_BOF일 경우, B+ tree 색인의 첫 번째 object (가장 작은 key 값을 갖는 leaf index entry) 를 검색함
        e = edubtm_FirstObject(root, kdesc, stopKval, stopCompOp, cursor);
    }
    else if(startCompOp == SM_EOF){
        // 2) 파라미터로 주어진 startCompOp가 SM_EOF일 경우, B+ tree 색인의 마지막 object (가장 큰 key 값을 갖는 leaf index entry) 를 검색함
        e = edubtm_LastObject(root, kdesc, stopKval, stopCompOp, cursor);
    }
    else{
        // 3) 이외의 경우, edubtm_Fetch()를 호출하여 B+ tree 색인에서 검색 조건을 만족하는 첫 번째 <object의 key, object ID> pair가 저장된 leaf index entry를 검색함
        e = edubtm_Fetch(root, kdesc, startKval, startCompOp, stopKval, stopCompOp, cursor);
    }

    // 4) 검색된 leaf index entry를 가리키는 cursor를 반환함
    if (e<0) ERR(e);
    return(eNOERROR);

} /* EduBtM_Fetch() */



/*@================================
 * edubtm_Fetch()
 *================================*/
/*
 * Function: Four edubtm_Fetch(PageID*, KeyDesc*, KeyVlaue*, Four, KeyValue*, Four, BtreeCursor*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Find the first object satisfying the given condition.
 *  This function handles only the following conditions:
 *  SM_EQ, SM_LT, SM_LE, SM_GT, SM_GE.
 *
 * Returns:
 *  Error code *   
 *    eBADCOMPOP_BTM
 *    eBADBTREEPAGE_BTM
 *    some errors caused by function calls
 */
Four edubtm_Fetch(
    PageID              *root,          /* IN The current root of the subtree */
    KeyDesc             *kdesc,         /* IN Btree key descriptor */
    KeyValue            *startKval,     /* IN key value of start condition */
    Four                startCompOp,    /* IN comparison operator of start condition */
    KeyValue            *stopKval,      /* IN key value of stop condition */
    Four                stopCompOp,     /* IN comparison operator of stop condition */
    BtreeCursor         *cursor)        /* OUT Btree Cursor */
{
    Four                e;              /* error number */
    Four                cmp;            /* result of comparison */
    Two                 idx;            /* index */
    PageID              child;          /* child page when the root is an internla page */
    Two                 alignedKlen;    /* aligned size of the key length */
    BtreePage           *apage;         /* a Page Pointer to the given root */
    BtreeOverflow       *opage;         /* a page pointer if it necessary to access an overflow page */
    Boolean             found;          /* search result */
    PageID              *leafPid;       /* leaf page pointed by the cursor */
    Two                 slotNo;         /* slot pointed by the slot */
    PageID              ovPid;          /* PageID of the overflow page */
    PageNo              ovPageNo;       /* PageNo of the overflow page */
    PageID              prevPid;        /* PageID of the previous page */
    PageID              nextPid;        /* PageID of the next page */
    ObjectID            *oidArray;      /* array of the ObjectIDs */
    Two                 iEntryOffset;   /* starting offset of an internal entry */
    btm_InternalEntry   *iEntry;        /* an internal entry */
    Two                 lEntryOffset;   /* starting offset of a leaf entry */
    btm_LeafEntry       *lEntry;        /* a leaf entry */
    Boolean             temp;

    /* Error check whether using not supported functionality by EduBtM */
    int i;
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    // 파라미터로 주어진 page를 root page로 하는 B+ tree 색인에서 검색 조건을 만족하는 첫 번째 <object의 key, object ID> pair가 저장된 leaf index entry를 검색하고, 검색된 leaf index entry를 가리키는 cursor를 반환함. 
    // (첫 번째 object는, 검색 조건을 만족하는 object들 중 검색 시작 key 값과 가장 가까운 key 값을 가지는 object를 의미함)

    e = BfM_GetTrain(root, &apage, PAGE_BUF);
	if (e!=eNOERROR) ERR(e);

	if (apage->any.hdr.type & INTERNAL) 
	{
        // 1) 파라미터로 주어진 root page가 internal page인 경우,
        // 1-1) 검색 조건을 만족하는 첫 번째 <object의 key, object ID> pair가 저장된 leaf page를 찾기 위해 다음으로 방문할 자식 page를 결정함
        // 1-2) 결정된 자식 page를 root page로 하는 B+ subtree에서 검색 조건을 만족하는 첫 번째 <object의 key, object ID> pair가 저장된 leaf index entry를 검색하기 위해 재귀적으로 edubtm_Fetch()를 호출함
        // 1-3) 검색된 leaf index entry를 가리키는 cursor를 반환함
		edubtm_BinarySearchInternal(&apage->bi, kdesc, startKval, &idx);

		if (idx < 0) {
            MAKE_PAGEID(child, root->volNo, apage->bi.hdr.p0);
		}
		else{
			iEntry = &(apage->bi.data[apage->bi.slot[-idx]]);
			MAKE_PAGEID(child, root->volNo, iEntry->spid);
        }
				
        edubtm_Fetch(&child, kdesc, startKval, startCompOp, stopKval, stopCompOp, cursor);
        BfM_FreeTrain(root, PAGE_BUF);
	}
	else if (apage->any.hdr.type & LEAF)
	{
        // 2) 파라미터로 주어진 root page가 leaf page인 경우,
        // 2-1) 검색 조건을 만족하는 첫 번째 <object의 key, object ID> pair가 저장된 index entry를 검색함
        // (검색 종료 연산자가 SM_LT, SM_LE, SM_GT, SM_GE, SM_EQ 중 하나일 때에만, 검색 시작 조건을 이용해 찾은 key 값과 검색 종료 조건을 비교하여 검색함)
        // 2-2) 검색된 index entry를 가리키는 cursor를 반환함
        leafPid = root;
		cursor->flag = CURSOR_ON;
	
		found = edubtm_BinarySearchLeaf(&apage->bl, kdesc, startKval, &slotNo);
		idx = slotNo;

        temp = FALSE;
		
        if (found == TRUE)
		{
			if (startCompOp == SM_EQ) {
                idx = slotNo;
            }
            else if (startCompOp == SM_LT) {   
                idx = slotNo-1;
            }
			else if (startCompOp == SM_LE) {
                idx = slotNo;
            }
			else if (startCompOp == SM_GT) {
                idx = slotNo+1;
            }
			else if (startCompOp == SM_GE) {
                idx = slotNo;
            }
		}
		else
		{
			if (startCompOp == SM_EQ) {
                cursor->flag = CURSOR_EOS;
            }
			else if (startCompOp == SM_LT) {
                idx = slotNo;
            }
			else if (startCompOp == SM_LE) {
                idx = slotNo;
            }
			else if (startCompOp == SM_GT) {
                idx = slotNo+1;
            }
			else if (startCompOp == SM_GE) {
                idx = slotNo+1;
            }
		}

		if (cursor->flag == CURSOR_EOS);
		else if (idx < 0)
		{
			if (apage->bl.hdr.prevPage != NIL)
			{
                MAKE_PAGEID(prevPid, root->volNo, apage->bl.hdr.prevPage);
                leafPid = &prevPid;

                temp = TRUE;
			}
			else
				cursor->flag = CURSOR_EOS;
			
		}
		else if (idx >= apage->bl.hdr.nSlots)
		{
			if (apage->bl.hdr.nextPage != NIL)
			{
				MAKE_PAGEID(nextPid, root->volNo, apage->bl.hdr.nextPage);
				leafPid = &nextPid;
				idx = 0;
			}
			else
				cursor->flag = CURSOR_EOS;
		}
        
        BfM_FreeTrain(root, PAGE_BUF);
        BfM_GetTrain(leafPid, &apage, PAGE_BUF);

		if (cursor->flag != CURSOR_EOS)
		{
            if(temp){
                idx = apage->bl.hdr.nSlots-1;
            }    

            lEntryOffset = apage->bl.slot[-idx];
            lEntry = &(apage->bl.data[lEntryOffset]);
			alignedKlen = ALIGNED_LENGTH(lEntry->klen);

			memcpy(&cursor->oid, &lEntry->kval + alignedKlen, sizeof(ObjectID));
			memcpy(&cursor->key, &lEntry->klen, sizeof(KeyValue));

			cursor->leaf = *leafPid;
			cursor->slotNo = idx;

            cmp = edubtm_KeyCompare(kdesc, &cursor->key, stopKval);
            if ((stopCompOp == SM_LT && !(cmp == LESS)) 
                || (stopCompOp == SM_LE && !(cmp == LESS || cmp == EQUAL)) 
                || (stopCompOp == SM_GT && !(cmp == GREATER)) 
                || (stopCompOp == SM_GE && !(cmp == GREATER || cmp == EQUAL)))
                cursor->flag = CURSOR_EOS;
        }	

		BfM_FreeTrain(leafPid, PAGE_BUF);
	}

    return(eNOERROR);
    
} /* edubtm_Fetch() */

