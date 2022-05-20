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
 * Module: edubtm_Compact.c
 * 
 * Description:
 *  Two functions edubtm_CompactInternalPage() and edubtm_CompactLeafPage() are
 *  used to compact the internal page and the leaf page, respectively.
 *
 * Exports:
 *  void edubtm_CompactInternalPage(BtreeInternal*, Two)
 *  void edubtm_CompactLeafPage(BtreeLeaf*, Two)
 */


#include <string.h>
#include "EduBtM_common.h"
#include "EduBtM_Internal.h"



/*@================================
 * edubtm_CompactInternalPage()
 *================================*/
/*
 * Function: edubtm_CompactInternalPage(BtreeInternal*, Two)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Reorganize the internal page to make sure the unused bytes in the page
 *  are located contiguously "in the middle", between the entries and the
 *  slot array. To compress out holes, entries must be moved toward the
 *  beginning of the page.
 *
 * Returns:
 *  None
 *
 * Side effects:
 *  The leaf page is reorganized to compact the space.
 */
void edubtm_CompactInternalPage(
    BtreeInternal       *apage,                 /* INOUT internal page to compact */
    Two                 slotNo)                 /* IN slot to go to the boundary of free space */
{
    BtreeInternal       tpage;                  /* temporay page used to save the given page */
    Two                 apageDataOffset;        /* where the next object is to be moved */
    Two                 len;                    /* length of the leaf entry */
    Two                 i;                      /* index variable */
    btm_InternalEntry   *entry;                 /* an entry in leaf page */

    // Internal page의 데이터 영역의 모든 자유 영역이 연속된 하나의 contiguous free area를 형성하도록 index entry들의 offset를 조정함

    // 1) 파라미터로 주어진 slotNo가 NIL이 아닌 경우, slotNo에 대응하는 index entry를 제외한 page의 모든 index entry들을 데이터 영역의 가장 앞부분부터 연속되게 저장함
    //     slotNo에 대응하는 index entry를 데이터 영역 상에서의 마지막 index entry로 저장함
    // 2) 파라미터로 주어진 slotNo가 NIL인 경우, Page의 모든 index entry들을 데이터 영역의 가장 앞부분부터 연속되게 저장함
    apageDataOffset = 0;
    for(i = 0; i < apage->hdr.nSlots; i++){

        if(i != slotNo){
            entry = apage->data + apage->slot[-i];
            len = sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(entry->klen);
            
            memcpy(tpage.data + apageDataOffset, entry, len);
            apage->slot[-i] = apageDataOffset;
            apageDataOffset += len;
        }
    }
    if(slotNo != NIL){
        entry = apage->data + apage->slot[-slotNo];
        len = sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(entry->klen);

        memcpy(tpage.data + apageDataOffset, entry, len);
        apage->slot[-slotNo] = apageDataOffset;
        apageDataOffset += len;
    }
    memcpy(apage->data, tpage.data, apageDataOffset);
    // 3) Page header를 갱신함
    apage->hdr.free = apageDataOffset;
    apage->hdr.unused = 0;

} /* edubtm_CompactInternalPage() */



/*@================================
 * edubtm_CompactLeafPage()
 *================================*/
/*
 * Function: void edubtm_CompactLeafPage(BtreeLeaf*, Two)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Reorganizes the leaf page to make sure the unused bytes in the page
 *  are located contiguously "in the middle", between the entries and the
 *  slot array. To compress out holes, entries must be moved toward the
 *  beginning of the page.
 *	
 * Return Values :
 *  None
 *
 * Side Effects :
 *  The leaf page is reorganized to comact the space.
 */
void edubtm_CompactLeafPage(
    BtreeLeaf 		*apage,			/* INOUT leaf page to compact */
    Two       		slotNo)			/* IN slot to go to the boundary of free space */
{	
    BtreeLeaf 		tpage;			/* temporay page used to save the given page */
    Two                 apageDataOffset;        /* where the next object is to be moved */
    Two                 len;                    /* length of the leaf entry */
    Two                 i;                      /* index variable */
    btm_LeafEntry 	*entry;			/* an entry in leaf page */
    Two 		alignedKlen;		/* aligned length of the key length */

    // Leaf page의 데이터 영역의 모든 자유 영역이 연속된 하나의 contiguous free area를 형성하도록 index entry들의 offset를 조정함

	apageDataOffset = 0;
    // 1) 파라미터로 주어진 slotNo가 NIL이 아닌 경우, slotNo에 대응하는 index entry를 제외한 page의 모든 index entry들을 데이터 영역의 가장 앞부분부터 연속되게 저장함
    //     slotNo에 대응하는 index entry를 데이터 영역 상에서의 마지막 index entry로 저장함
    // 2) 파라미터로 주어진 slotNo가 NIL인 경우, Page의 모든 index entry들을 데이터 영역의 가장 앞부분부터 연속되게 저장함
    for (i=0; i<apage->hdr.nSlots; i++)
	{
		if (i != slotNo)
		{
			entry = apage->data + apage->slot[-i];
			len = 2 * sizeof(Two) + ALIGNED_LENGTH(entry->klen) + OBJECTID_SIZE;
			memcpy(tpage.data + apageDataOffset, entry, len);
			apage->slot[-i] = apageDataOffset;
			apageDataOffset += len;
		}
	}
    if (slotNo != NIL)
	{
		entry = apage->data + apage->slot[-slotNo];
		len = 2 * sizeof(Two) + ALIGNED_LENGTH(entry->klen) + OBJECTID_SIZE;
		memcpy(tpage.data + apageDataOffset, entry, len);
		apage->slot[-slotNo] = apageDataOffset;
		apageDataOffset += len;
	}
    memcpy(apage->data, tpage.data, apageDataOffset);
    // 3) Page header를 갱신함
    apage->hdr.free = apageDataOffset;
	apage->hdr.unused = 0;

} /* edubtm_CompactLeafPage() */
