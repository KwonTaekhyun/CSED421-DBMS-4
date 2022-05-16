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
 * Module: edubtm_LastObject.c
 *
 * Description : 
 *  Find the last ObjectID of the given Btree.
 *
 * Exports:
 *  Four edubtm_LastObject(PageID*, KeyDesc*, KeyValue*, Four, BtreeCursor*) 
 */


#include <string.h>
#include "EduBtM_common.h"
#include "BfM.h"
#include "EduBtM_Internal.h"



/*@================================
 * edubtm_LastObject()
 *================================*/
/*
 * Function:  Four edubtm_LastObject(PageID*, KeyDesc*, KeyValue*, Four, BtreeCursor*) 
 *
 * Description : 
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Find the last ObjectID of the given Btree. The 'cursor' will indicate
 *  the last ObjectID in the Btree, and it will be used as successive access
 *  by using the Btree.
 *
 * Returns:
 *  error code
 *    eBADPAGE_BTM
 *    some errors caused by function calls
 *
 * Side effects:
 *  cursor : the last ObjectID and its position in the Btree
 */
Four edubtm_LastObject(
    PageID   		*root,		/* IN the root of Btree */
    KeyDesc  		*kdesc,		/* IN key descriptor */
    KeyValue 		*stopKval,	/* IN key value of stop condition */
    Four     		stopCompOp,	/* IN comparison operator of stop condition */
    BtreeCursor 	*cursor)	/* OUT the last BtreeCursor to be returned */
{
    int			i;
    Four 		e;		/* error number */
    Four 		cmp;		/* result of comparison */
    BtreePage 		*apage;		/* pointer to the buffer holding current page */
    BtreeOverflow 	*opage;		/* pointer to the buffer holding overflow page */
    PageID 		curPid;		/* PageID of the current page */
    PageID 		child;		/* PageID of the child page */
    PageID 		ovPid;		/* PageID of the current overflow page */
    PageID 		nextOvPid;	/* PageID of the next overflow page */
    Two 		lEntryOffset;	/* starting offset of a leaf entry */
    Two 		iEntryOffset;	/* starting offset of an internal entry */
    btm_LeafEntry 	*lEntry;	/* a leaf entry */
    btm_InternalEntry 	*iEntry;	/* an internal entry */
    Four 		alignedKlen;	/* aligned length of the key length */
        

    if (root == NULL) ERR(eBADPAGE_BTM);

    /* Error check whether using not supported functionality by EduBtM */
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }
    
    // B+ tree 색인에서 마지막 object (가장 큰 key값을 갖는 leaf index entry) 를 검색함

    // 1) B+ tree 색인의 마지막 leaf page의 마지막 index entry (slot 번호 = nSlots - 1) 를 가리키는 cursor를 반환함
    e = BfM_GetTrain(root, &apage, PAGE_BUF);
	if (e < 0) ERR(e);

    if (apage->any.hdr.type & INTERNAL)
	{
		MAKE_PAGEID(child, root->volNo, apage->bi.hdr.p0);
		edubtm_LastObject(&child, kdesc, stopKval, stopCompOp, cursor);
	}
	else if (apage->any.hdr.type & LEAF)
	{
        if (apage->bl.hdr.nextPage != NIL)
		{
			MAKE_PAGEID(curPid, root->volNo, apage->bl.hdr.nextPage);
			edubtm_LastObject(&curPid, kdesc, stopKval, stopCompOp, cursor);
		}
		else
		{
			lEntry = apage->bl.data + apage->bl.slot[-1*(apage->bl.hdr.nSlots-1)];
			alignedKlen = ALIGNED_LENGTH(lEntry->klen);
		
			memcpy(&cursor->key, &lEntry->klen, sizeof(KeyValue));

            cmp = edubtm_KeyCompare(kdesc, stopKval, &cursor->key);

            if(cmp == GREAT || (cmp == EQUAL && stopCompOp == SM_GT)){
                cursor->flag = CURSOR_EOS;
            }
            else {
                cursor->flag = CURSOR_ON;
                cursor->leaf = *root;
                cursor->slotNo = apage->bl.hdr.nSlots-1;

                memcpy(&cursor->oid, &lEntry->kval + alignedKlen, sizeof(ObjectID));
            }
		}
	}

	e = BfM_FreeTrain(root, PAGE_BUF);
	if (e < 0) ERR(e);
    // 2) 검색 종료 key 값이 마지막 object의 key 값 보다 크거나, key 값은 같으나 검색 종료 연산이 SM_GT인 경우 CURSOR_EOS 반환 

    return(eNOERROR);
    
} /* edubtm_LastObject() */
