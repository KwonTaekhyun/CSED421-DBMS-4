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
 * Module: edubtm_BinarySearch.c
 *
 * Description :
 *  This file has three function about searching. All these functions use the
 *  binary search algorithm. If the entry is found successfully, it returns 
 *  TRUE and the correct position as an index, otherwise, it returns FALSE and
 *  the index whose key value is the largest in the given page but less than
 *  the given key value in the function edubtm_BinarSearchInternal; in the
 *  function edubtm_BinarySearchLeaf() the index whose key value is the smallest
 *  in the given page but larger than the given key value.
 *
 * Exports:
 *  Boolean edubtm_BinarySearchInternal(BtreeInternal*, KeyDesc*, KeyValue*, Two*)
 *  Boolean edubtm_BinarySearchLeaf(BtreeLeaf*, KeyDesc*, KeyValue*, Two*)
 */


#include "EduBtM_common.h"
#include "EduBtM_Internal.h"



/*@================================
 * edubtm_BinarySearchInternal()
 *================================*/
/*
 * Function:  Boolean edubtm_BinarySearchInternal(BtreeInternal*, KeyDesc*,
 *                                             KeyValue*, Two*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Search the internal entry of which value equals to or less than the given
 *  key value.
 *
 * Returns:
 *  Result of search: TRUE if the same key is found, otherwise FALSE
 *
 * Side effects:
 *  1) parameter idx : slot No of the slot having the key equal to or
 *                     less than the given key value
 *                     
 */
Boolean edubtm_BinarySearchInternal(
    BtreeInternal 	*ipage,		/* IN Page Pointer to an internal page */
    KeyDesc       	*kdesc,		/* IN key descriptor */
    KeyValue      	*kval,		/* IN key value */
    Two          	*idx)		/* OUT index to be returned */
{
    Two  		low;		/* low index */
    Two  		mid;		/* mid index */
    Two  		high;		/* high index */
    Four 		cmp;		/* result of comparison */
    btm_InternalEntry 	*entry;	/* an internal entry */

    
    /* Error check whether using not supported functionality by EduBtM */
    int i;
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }
    
    // Internal page에서 파라미터로 주어진 key 값보다 작거나 같은 key 값을 갖는 index entry를 검색하고, 검색된 index entry의 위치 (slot 번호) 를 반환함

    if(ipage->hdr.nSlots == 0)
	{	
		*idx = -1;
		return FALSE;
	}
    // 1) 파라미터로 주어진 key 값과 같은 key 값을 갖는 index entry가 존재하는 경우, 해당 index entry의 slot 번호 및 TRUE를 반환함
    // 2) 파라미터로 주어진 key 값과 같은 key 값을 갖는 index entry가 존재하지 않는 경우, 파라미터로 주어진 key 값보다 작은 key 값을 갖는 index entry들 중 가장 큰 key 값을 갖는 index entry의 slot 번호 및 FALSE를 반환함. 주어진 key 값보다 작은 key 값을 갖는 entry가 없을 경우 slot 번호로 -1을 반환함.
    low = 0;
	high = ipage->hdr.nSlots-1;

    while (low <= high) 
	{
		mid = (low + high)/2;

		entry = &(ipage->data[ipage->slot[-mid]]);
		cmp = edubtm_KeyCompare(kdesc, &entry->klen, kval);

        if (cmp == LESS) 		
            low = mid+1;
		else if (cmp == EQUAL) 	
            break;
		else 					
            high = mid-1;
	}

	if (cmp == EQUAL)
	{
		*idx = mid;
		return TRUE;
	}
	else
	{
        while((cmp != LESS)&&(mid>=0)){
            mid--;
            entry = &(ipage->data[ipage->slot[-mid]]);
		    cmp = edubtm_KeyCompare(kdesc, &entry->klen, kval);
        }

        *idx = mid;
        return FALSE;
	}
    
} /* edubtm_BinarySearchInternal() */



/*@================================
 * edubtm_BinarySearchLeaf()
 *================================*/
/*
 * Function: Boolean edubtm_BinarySearchLeaf(BtreeLeaf*, KeyDesc*,
 *                                        KeyValue*, Two*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Search the leaf item of which value equals to or less than the given
 *  key value.
 *
 * Returns:
 *  Result of search: TRUE if the same key is found, FALSE otherwise
 *
 * Side effects:
 *  1) parameter idx: slot No of the slot having the key equal to or
 *                    less than the given key value
 */
Boolean edubtm_BinarySearchLeaf(
    BtreeLeaf 		*lpage,		/* IN Page Pointer to a leaf page */
    KeyDesc   		*kdesc,		/* IN key descriptor */
    KeyValue  		*kval,		/* IN key value */
    Two       		*idx)		/* OUT index to be returned */
{
    Two  		low;		/* low index */
    Two  		mid;		/* mid index */
    Two  		high;		/* high index */
    Four 		cmp;		/* result of comparison */
    btm_LeafEntry 	*entry;		/* a leaf entry */


    /* Error check whether using not supported functionality by EduBtM */
    int i;
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    // Leaf page에서 파라미터로 주어진 key 값보다 작거나 같은 key 값을 갖는 index entry를 검색하고, 검색된 index entry의 위치 (slot 번호) 를 반환함

    if(lpage->hdr.nSlots == 0)
	{	
		*idx = -1;
		return FALSE;
	}
    // 1) 파라미터로 주어진 key 값과 같은 key 값을 갖는 index entry가 존재하는 경우, 해당 index entry의 slot 번호 및 TRUE를 반환함
    // 2) 파라미터로 주어진 key 값과 같은 key 값을 갖는 index entry가 존재하지 않는 경우, 파라미터로 주어진 key 값보다 작은 key 값을 갖는 index entry들 중 가장 큰 key 값을 갖는 index entry의 slot 번호 및 FALSE를 반환함. 주어진 key 값보다 작은 key 값을 갖는 entry가 없을 경우 slot 번호로 -1을 반환함.
    low = 0;
	high = lpage->hdr.nSlots-1;

    while (low <= high) 
	{
		mid = (low + high)/2;

		entry = &(lpage->data[lpage->slot[-mid]]);
		cmp = edubtm_KeyCompare(kdesc, &entry->klen, kval);

        if (cmp == LESS) 		
            low = mid+1;
		else if (cmp == EQUAL) 	
            break;
		else 					
            high = mid-1;
	}

	if (cmp == EQUAL)
	{
		*idx = mid;
		return TRUE;
	}
	else
	{
        while((cmp != LESS)&&(mid>=0)){
            mid--;
            entry = &(lpage->data[lpage->slot[-mid]]);
		    cmp = edubtm_KeyCompare(kdesc, &entry->klen, kval);
        }

        *idx = mid;
        return FALSE;
	}
    
} /* edubtm_BinarySearchLeaf() */
