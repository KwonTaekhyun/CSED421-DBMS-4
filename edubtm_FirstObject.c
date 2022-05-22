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
 * Module: edubtm_FirstObject.c
 *
 * Description : 
 *  Find the first ObjectID of the given Btree. 
 *
 * Exports:
 *  Four edubtm_FirstObject(PageID*, KeyDesc*, KeyValue*, Four, BtreeCursor*)
 */


#include <string.h>
#include "EduBtM_common.h"
#include "BfM.h"
#include "EduBtM_Internal.h"



/*@================================
 * edubtm_FirstObject()
 *================================*/
/*
 * Function: Four edubtm_FirstObject(PageID*, KeyDesc*, KeyValue*, Four, BtreeCursor*)
 *
 * Description : 
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Find the first ObjectID of the given Btree. The 'cursor' will indicate
 *  the first ObjectID in the Btree, and it will be used as successive access
 *  by using the Btree.
 *
 * Returns:
 *  error code
 *    eBADPAGE_BTM
 *    some errors caused by function calls
 *
 * Side effects:
 *  cursor : A position in the Btree which indicates the first ObjectID.
 *             The first object's object identifier is also returned via this.
 */
Four edubtm_FirstObject(
    PageID  		*root,		/* IN The root of Btree */
    KeyDesc 		*kdesc,		/* IN Btree key descriptor */
    KeyValue 		*stopKval,	/* IN key value of stop condition */
    Four     		stopCompOp,	/* IN comparison operator of stop condition */
    BtreeCursor 	*cursor)	/* OUT The first ObjectID in the Btree */
{
    int			i;
    Four 		e;		/* error */
    Four 		cmp;		/* result of comparison */
    PageID 		curPid;		/* PageID of the current page */
    PageID 		child;		/* PageID of the child page */
    BtreePage 		*apage;		/* a page pointer */
    Two                 lEntryOffset;   /* starting offset of a leaf entry */
    btm_LeafEntry 	*lEntry;	/* a leaf entry */
    Two                 alignedKlen;    /* aligned length of the key length */
    

    if (root == NULL) ERR(eBADPAGE_BTM);

    /* Error check whether using not supported functionality by EduBtM */
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    // B+ tree 색인에서 첫 번째 object (가장 작은 key 값을 갖는 leaf index entry) 를 검색함

    // 1) B+ tree 색인의 첫 번째 leaf page의 첫 번째 leaf index entry를 가리키는 cursor를 반환함
    BfM_GetTrain(root, &apage, PAGE_BUF);

    if (apage->any.hdr.type & INTERNAL)
	{
		MAKE_PAGEID(child, root->volNo, apage->bi.hdr.p0);
		edubtm_FirstObject(&child, kdesc, stopKval, stopCompOp, cursor);
	}
	else if (apage->any.hdr.type & LEAF)
	{
        lEntry = &(apage->bl.data[apage->bl.slot[0]]);
		
		memcpy(&cursor->key, &lEntry->klen, sizeof(KeyValue));

        cmp = edubtm_KeyCompare(kdesc, stopKval, &cursor->key);

        if(cmp == LESS || (cmp == EQUAL && stopCompOp == SM_LT)){
            cursor->flag = CURSOR_EOS;
        }
        else {
            cursor->flag = CURSOR_ON;
            cursor->leaf = *root;
            cursor->slotNo = 0;
            cursor->oid = ((ObjectID *)&lEntry->kval[ALIGNED_LENGTH(cursor->key.len)])[0];
        }
	}
    // 2) 검색 종료 key 값이 첫 번째 object의 key 값 보다 작거나, key 값은 같으나 검색 종료 연산이 SM_LT인 경우 CURSOR_EOS 반환 
    BfM_FreeTrain(root, PAGE_BUF);

    return(eNOERROR);
    
} /* edubtm_FirstObject() */
