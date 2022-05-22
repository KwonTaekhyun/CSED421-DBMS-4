# EduBtM Report

Name: 권택현

Student id: 20180522

# Problem Analysis

EduCOSMOS project의 B+ tree 색인 manager에 대한 연산들을 구현을 목표로 하며
구체적으로는 B+ tree 색인 및 색인 page 관련 구조에 대한 연산들을 구현할 것이다.
이는 오디세우스/COSMOS BtM의 기능들 중 극히 제한된 일부 기능들만을 구현한 것이다.

# Design For Problem Solving

## High Level

1. EduBtM_CreateIndex

    색인 file에서 새로운 B+ tree 색인을 생성하고, 생성된 색인의 root page의 page ID를 반환함

    1) btm_AllocPage()를 호출하여 색인 file의 첫 번째 page를 할당 받음
    2) 할당 받은 page를 root page로 초기화함
    3) 초기화된 root page의 page ID를 반환함

2. EduBtM_DropIndex

    색인 file에서 B+ tree 색인을 삭제함

    1) B+ tree 색인의 root page 및 모든 자식 page들을 각각 deallocate 함

3. EduBtM_Fetch

    B+ tree 색인에서 검색 조건을 만족하는 첫 번째 object를 검색하고, 검색된 object를 가리키는 cursor를 반환함

    1) 파라미터로 주어진 startCompOp가 SM_BOF일 경우, B+ tree 색인의 첫 번째 object (가장 작은 key 값을 갖는 leaf index entry) 를 검색함
    2) 파라미터로 주어진 startCompOp가 SM_EOF일 경우, B+ tree 색인의 마지막 object (가장 큰 key 값을 갖는 leaf index entry) 를 검색함
    3) 이외의 경우, edubtm_Fetch()를 호출하여 B+ tree 색인에서 검색 조건을 만족하는 첫 번째 <object의 key, object ID> pair가 저장된 leaf index entry를 검색함
    4) 검색된 leaf index entry를 가리키는 cursor를 반환함

4. EduBtM_FetchNext

    B+ tree 색인에서 검색 조건을 만족하는 현재 object의 다음 object를 검색하고, 검색된 object를 가리키는 cursor를 반환함

    1) edubtm_FetchNext()를 호출하여 B+ tree 색인에서 검색 조건을 만족하는 현재 leaf index entry의 다음 leaf index entry를 검색함
    2) 검색된 leaf index entry를 가리키는 cursor를 반환함

5. EduBtM_InsertObject

    B+ tree 색인에 새로운 object를 삽입함

    1) edubtm_Insert()를 호출하여 새로운 object에 대한 <object의 key, object ID> pair를 B+ tree 색인에 삽입함
    2) Root page에서 split이 발생하여 새로운 root page 생성이 필요한 경우, edubtm_root_insert()를 호출하여 이를 처리함

6. EduBtM_DeleteObject

    B+ tree 색인에서 object를 삭제함

    1) edubtm_Delete()를 호출하여 삭제할 object에 대한 <object의 key, object ID> pair를 B+ tree 색인에서 삭제함
    2) Root page에서 underflow가 발생한 경우, btm_root_delete()를 호출하여 이를 처리함. 
        btm_root_delete()는 underflow가 발생한 root page가 비어있는지를 확인하여 비어있는 경우 root page를 삭제함
    3) Root page에서 split이 발생한 경우, edubtm_root_insert()를 호출하여 이를 처리함
        Redistribute 과정 중에 root page의 index entry가 length가 더 긴 index entry로 교체되었을 수 있으므로, root page에서 split이 발생할 수 있음

## Low Level

1. edubtm_InitLeaf

    Page를 B+ tree 색인의 leaf page로 초기화함

    1) Page header를 leaf page로 초기화함

2. edubtm_InitInternal

    Page를 B+ tree 색인의 internal page로 초기화함

    1) Page header를 internal page로 초기화함

3. edubtm_FreePages

    B+ tree 색인 page를 deallocate 함

    1) 파라미터로 주어진 page의 모든 자식 page들에 대해 재귀적으로 edubtm_FreePages()를 호출하여 해당 page들을 deallocate 함
    2) 파라미터로 주어진 page를 deallocate 함
        2-1) Page header의 type에서 해당 page가 deallocate 될 page임을 나타내는 bit를 set 및 나머지 bit들을 unset 함
        2-2) 해당 page를 deallocate 함

4. edubtm_Fetch

    파라미터로 주어진 page를 root page로 하는 B+ tree 색인에서 검색 조건을 만족하는 첫 번째 <object의 key, object ID> pair가 저장된 leaf index entry를 검색하고, 검색된 leaf index entry를 가리키는 cursor를 반환함. 
    (첫 번째 object는, 검색 조건을 만족하는 object들 중 검색 시작 key 값과 가장 가까운 key 값을 가지는 object를 의미함)

    1) 파라미터로 주어진 root page가 internal page인 경우,
        1-1) 검색 조건을 만족하는 첫 번째 <object의 key, object ID> pair가 저장된 leaf page를 찾기 위해 다음으로 방문할 자식 page를 결정함
        1-2) 결정된 자식 page를 root page로 하는 B+ subtree에서 검색 조건을 만족하는 첫 번째 <object의 key, object ID> pair가 저장된 leaf index entry를 검색하기 위해 재귀적으로 edubtm_Fetch()를 호출함
        1-3) 검색된 leaf index entry를 가리키는 cursor를 반환함
    2) 파라미터로 주어진 root page가 leaf page인 경우,
        2-1) 검색 조건을 만족하는 첫 번째 <object의 key, object ID> pair가 저장된 index entry를 검색함
        (검색 종료 연산자가 SM_LT, SM_LE, SM_GT, SM_GE, SM_EQ 중 하나일 때에만, 검색 시작 조건을 이용해 찾은 key 값과 검색 종료 조건을 비교하여 검색함)
        2-2) 검색된 index entry를 가리키는 cursor를 반환함

5. edubtm_FetchNext

    B+ tree 색인에서 검색 조건을 만족하는 현재 leaf index entry의 다음 leaf index entry를 검색하고, 검색된 leaf index entry를 가리키는 cursor를 반환함. 검색 조건이 SM_GT, SM_GE, SM_BOF일 경우 key 값이 작아지는 방향으로 backward scan을 하며, 그 외의 경우 key 값이 커지는 방향으로 forward scan을 한다

    1) 검색 조건을 만족하는 다음 leaf index entry를 검색함
    2) 검색된 leaf index entry를 가리키는 cursor를 반환함

6. edubtm_FirstObject

    B+ tree 색인에서 첫 번째 object (가장 작은 key 값을 갖는 leaf index entry) 를 검색함

    1) B+ tree 색인의 첫 번째 leaf page의 첫 번째 leaf index entry를 가리키는 cursor를 반환함
    2) 검색 종료 key 값이 첫 번째 object의 key 값 보다 작거나, key 값은 같으나 검색 종료 연산이 SM_LT인 경우 CURSOR_EOS 반환 

7. edubtm_LastObject

    B+ tree 색인에서 마지막 object (가장 큰 key값을 갖는 leaf index entry) 를 검색함

    1) B+ tree 색인의 마지막 leaf page의 마지막 index entry (slot 번호 = nSlots - 1) 를 가리키는 cursor를 반환함
    2) 검색 종료 key 값이 마지막 object의 key 값 보다 크거나, key 값은 같으나 검색 종료 연산이 SM_GT인 경우 CURSOR_EOS 반환 

8. edubtm_BinarySearchLeaf

    Leaf page에서 파라미터로 주어진 key 값보다 작거나 같은 key 값을 갖는 index entry를 검색하고, 검색된 index entry의 위치 (slot 번호) 를 반환함

    1) 파라미터로 주어진 key 값과 같은 key 값을 갖는 index entry가 존재하는 경우, 해당 index entry의 slot 번호 및 TRUE를 반환함
    2) 파라미터로 주어진 key 값과 같은 key 값을 갖는 index entry가 존재하지 않는 경우, 파라미터로 주어진 key 값보다 작은 key 값을 갖는 index entry들 중 가장 큰 key 값을 갖는 index entry의 slot 번호 및 FALSE를 반환함. 주어진 key 값보다 작은 key 값을 갖는 entry가 없을 경우 slot 번호로 -1을 반환함.

9. edubtm_BinarySearchInternal

    Internal page에서 파라미터로 주어진 key 값보다 작거나 같은 key 값을 갖는 index entry를 검색하고, 검색된 index entry의 위치 (slot 번호) 를 반환함

    1) 파라미터로 주어진 key 값과 같은 key 값을 갖는 index entry가 존재하는 경우, 해당 index entry의 slot 번호 및 TRUE를 반환함
    2) 파라미터로 주어진 key 값과 같은 key 값을 갖는 index entry가 존재하지 않는 경우, 파라미터로 주어진 key 값보다 작은 key 값을 갖는 index entry들 중 가장 큰 key 값을 갖는 index entry의 slot 번호 및 FALSE를 반환함. 주어진 key 값보다 작은 key 값을 갖는 entry가 없을 경우 slot 번호로 -1을 반환함.

10. edubtm_KeyCompare

    파라미터로 주어진 두 key 값의 대소를 비교하고, 비교 결과를 반환함. Variable length string의 경우 사전식 순서를 이용하여 비교함

    1) 두 key 값이 같은 경우, EQUAL을 반환함
    2) 첫 번째 key 값이 큰 경우, GREATER를 반환함
    3) 첫 번째 key 값이 작은 경우, LESS를 반환함

11. edubtm_Insert

    파라미터로 주어진 page를 root page로 하는 B+ tree 색인에 새로운 object에 대한 <object의 key, object ID> pair를 삽입하고, root page에서 split이 발생한 경우, split으로 생성된 새로운 page를 가리키는 internal index entry를 반환함

    1) 파라미터로 주어진 root page가 internal page인 경우
        1-1) 새로운 <object의 key, object ID> pair를 삽입할 leaf page를 찾기 위해 다음으로 방문할 자식 page를 결정함
        1-2) 결정된 자식 page를 root page로 하는 B+ subtree에 새로운 <object의 key, object ID> pair를 삽입하기 위해 재귀적으로 edubtm_Insert()를 호출함
        1-3) 결정된 자식 page에서 split이 발생한 경우, 해당 split으로 생성된 새로운 page를 가리키는 internal index entry를 파라미터로 주어진 root page에 삽입함
        1-4) 파라미터로 주어진 root page에서 split이 발생한 경우, 해당 split으로 생성된 새로운 page를 가리키는 internal index entry를 반환함
    2) 파라미터로 주어진 root page가 leaf page인 경우
        2-1) edubtm_InsertLeaf()를 호출하여 해당 page에 새로운 <object의 key, object ID> pair를 삽입함
        2-2) Split이 발생한 경우, 해당 split으로 생성된 새로운 page를 가리키는 internal index entry를 반환함

12. edubtm_root_insert

    Root page가 split 된 B+ tree 색인을 위한 새로운 root page를 생성함

    1) 새로운 page를 할당 받음
    2) 기존 root page를 할당 받은 page로 복사함
    3) 기존 root page를 새로운 root page로서 초기화함 (B+ tree 색인의 root page의 page ID를 일관되게 유지하기 위함)
    4) 할당 받은 page와 root page split으로 생성된 page가 새로운 root page의 자식 page들이 되도록 설정함
        4-1) Split으로 생성된 page를 가리키는 internal index entry를 새로운 root page에 삽입함
        4-2) 새로운 root page의 header의 p0 변수에 할당 받은 page의 번호를 저장함
        4-3) 새로운 root page의 두 자식 page들이 leaf인 경우, 두 자식 page들간의 doubly linked list를 설정함
            (Split으로 생성된 page가 할당 받은 page의 다음 page가 되도록 설정함)

13. edubtm_InsertInternal

    Internal page에 새로운 index entry를 삽입하고, split이 발생한 경우, split으로 생성된 새로운 internal page를 가리키는 internal index entry를 반환함

    1) 새로운 index entry 삽입을 위해 필요한 자유 영역의 크기를 계산함
    2) Page에 여유 영역이 있는 경우,
        2-1) 필요시 page를 compact 함
        2-2) 파라미터로 주어진 slot 번호의 다음 slot 번호로 새로운 index entry를 삽입함
    3) Page에 여유 영역이 없는 경우 (page overflow),
        3-1) edubtm_SplitInternal()을 호출하여 page를 split 함
        3-2) Split으로 생성된 새로운 internal page를 가리키는 internal index entry를 반환함

14. edubtm_InsertLeaf

    Leaf page에 새로운 index entry를 삽입하고, split이 발생한 경우, split으로 생성된 새로운 leaf page를 가리키는 internal index entry를 반환함

    1) 새로운 index entry의 삽입 위치 (slot 번호) 를 결정함
    2) 새로운 index entry 삽입을 위해 필요한 자유 영역의 크기를 계산함
    3) Page에 여유 영역이 있는 경우,
        3-1) 필요시 page를 compact 함
        3-2) 결정된 slot 번호로 새로운 index entry를 삽입함
    4) Page에 여유 영역이 없는 경우 (page overflow),
        4-1) edubtm_SplitLeaf()를 호출하여 page를 split 함
        4-2) Split으로 생성된 새로운 leaf page를 가리키는 internal index entry를 반환함

15. edubtm_SplitLeaf

    Overflow가 발생한 leaf page를 split 하여 파라미터로 주어진 index entry를 삽입하고, split으로 생성된 새로운 leaf page를 가리키는 internal index entry를 반환함

    1) 새로운 page를 할당 받음
    2) 할당 받은 page를 leaf page로 초기화함
    3) 기존 index entry들 및 삽입할 index entry를 key 순으로 정렬하여 overflow가 발생한 page 및 할당 받은 page에 나누어 저장함
    4) 할당 받은 page를 leaf page들간의 doubly linked list에 추가함
    5) 할당 받은 page를 가리키는 internal index entry를 생성함
    6) Split된 page가 ROOT일 경우, type을 LEAF로 변경함
    7) 생성된 index entry를 반환함

16. edubtm_SplitInternal

    Overflow가 발생한 internal page를 split 하여 파라미터로 주어진 index entry를 삽입하고, split으로 생성된 새로운 internal page를 가리키는 internal index entry를 반환함

    1) 새로운 page를 할당 받음
    2) 할당 받은 page를 internal page로 초기화함
    3) 기존 index entry들 및 삽입할 index entry를 key 순으로 정렬하여 overflow가 발생한 page 및 할당 받은 page에 나누어 저장함
    3-1) 먼저, overflow가 발생한 page에 데이터 영역을 50% 이상 채우는 수의 index entry들을 저장함
    3-2) 할당 받은 page의 header의 p0 변수에 아직 저장되지 않은 index entry 중 첫 번째 index entry (1st entry) 가 가리키는 자식 page의 번호를 저장함
    3-3) 1st entry는 할당 받은 page를 가리키는 internal index entry로 설정하여 반환함 (자식 page의 번호 := 할당 받은 page의 번호)
    3-4) 나머지 index entry들을 할당 받은 page에 저장함
    3-5) 각 page의 header를 갱신함
    4) Split된 page가 ROOT일 경우, type을 INTERNAL로 변경함

17. edubtm_CompactLeafPage

    Leaf page의 데이터 영역의 모든 자유 영역이 연속된 하나의 contiguous free area를 형성하도록 index entry들의 offset를 조정함

    1) 파라미터로 주어진 slotNo가 NIL이 아닌 경우, slotNo에 대응하는 index entry를 제외한 page의 모든 index entry들을 데이터 영역의 가장 앞부분부터 연속되게 저장함
        slotNo에 대응하는 index entry를 데이터 영역 상에서의 마지막 index entry로 저장함
    2) 파라미터로 주어진 slotNo가 NIL인 경우, Page의 모든 index entry들을 데이터 영역의 가장 앞부분부터 연속되게 저장함
    3) Page header를 갱신함

18. edubtm_CompactInternalPage

    Internal page의 데이터 영역의 모든 자유 영역이 연속된 하나의 contiguous free area를 형성하도록 index entry들의 offset를 조정함

    1) 파라미터로 주어진 slotNo가 NIL이 아닌 경우, slotNo에 대응하는 index entry를 제외한 page의 모든 index entry들을 데이터 영역의 가장 앞부분부터 연속되게 저장함
        slotNo에 대응하는 index entry를 데이터 영역 상에서의 마지막 index entry로 저장함
    2) 파라미터로 주어진 slotNo가 NIL인 경우, Page의 모든 index entry들을 데이터 영역의 가장 앞부분부터 연속되게 저장함
    3) Page header를 갱신함

19. edubtm_Delete

    파라미터로 주어진 page를 root page로 하는 B+ tree 색인에서 <object의 key, object ID> pair를 삭제함

    1) 파라미터로 주어진 root page가 internal page인 경우
    1-1) 삭제할 <object의 key, object ID> pair가 저장된 leaf page를 찾기 위해 다음으로 방문할 자식 page를 결정함
    1-2) 결정된 자식 page를 root page로 하는 B+ subtree에서 <object의 key, object ID> pair를 삭제하기 위해 재귀적으로 edubtm_Delete()를 호출함
    1-3) 결정된 자식 page에서 underflow가 발생한 경우, btm_Underflow()를 호출하여 이를 처리함
        Underflow가 발생한 자식 page의 부모 page (파라미터로 주어진 root page) 에서 overflow가 발생한 경우, edubtm_InsertInternal()을 호출하여 overflow로 인해 삽입되지 못한 internal index entry를 부모 page에 삽입함. edubtm_InsertInternal() 호출 결과로서 부모 page가 split 되므로, out parameter인 h를 TRUE로 설정하고 split으로 생성된 새로운 page를 가리키는 internal index entry를 반환함
        btm_Underflow() 호출 결과로서 파라미터로 주어진 root page의 내용이 변경되므로, btm_Underflow() 호출 후 root page의 DIRTY bit를 1로 set 해야 함
    2) 파라미터로 주어진 root page가 leaf page인 경우,
    2-1) edubtm_DeleteLeaf()를 호출하여 해당 page에서 <object의 key, object ID> pair를 삭제함
    2-2) 해당 page에서 underflow가 발생한 경우 (page의 data 영역 중 자유 영역의 크기 > (page의 data 영역의 전체 크기 / 2)), out parameter인 f를 TRUE로 설정함

20. edubtm_DeleteLeaf

    Leaf page에서 <object의 key, object ID> pair를 삭제함

    1) 삭제할 <object의 key, object ID> pair가 저장된 index entry의 offset이 저장된 slot을 삭제함
        Slot array 중간에 삭제된 빈 slot이 없도록 slot array를 compact 함
    2) Leaf page의 header를 갱신함
    3) Leaf page에서 underflow가 발생한 경우(page의 data 영역 중 자유 영역의 크기 > (page의 data 영역의 전체 크기 / 2)), out parameter인 f를 TRUE로 설정함

# Mapping Between Implementation And the Design

<High level>

1. EduBtM_CreateIndex

```cpp
Four EduBtM_CreateIndex(
    ObjectID *catObjForFile,	/* IN catalog object of B+ tree file */
    PageID *rootPid)		/* OUT root page of the newly created B+tree */
{
    Four e;			/* error number */
    Boolean isTmp;
    SlottedPage *catPage;	/* buffer page containing the catalog object */
    sm_CatOverlayForBtree *catEntry; /* pointer to Btree file catalog information */
    PhysicalFileID pFid;	/* physical file ID */

    //색인 file에서 새로운 B+ tree 색인을 생성하고, 생성된 색인의 root page의 page ID를 반환한다

    // 1) btm_AllocPage()를 호출하여 색인 file의 첫 번째 page를 할당 받음
    e = BfM_GetTrain((TrainID*)catObjForFile, (char**)&catPage, PAGE_BUF);
    if (e < 0) ERR(e);

    GET_PTR_TO_CATENTRY_FOR_BTREE(catObjForFile, catPage, catEntry);
    MAKE_PAGEID(pFid, catEntry->fid.volNo, catEntry->firstPage);

    e = btm_AllocPage(catObjForFile, (PageID*)&pFid, rootPid);
    if (e<0) ERR(e);
    // 2) 할당 받은 page를 root page로 초기화함
    e = edubtm_InitLeaf(rootPid, TRUE, FALSE);
    if (e<0) ERR(e);

    e = BfM_FreeTrain((TrainID*)catObjForFile, PAGE_BUF);
    if (e<0) ERR(e);
    // 3) 초기화된 root page의 page ID를 반환함
    // ID를 반환함

    return(eNOERROR);
    
} /* EduBtM_CreateIndex() */
```

2. EduBtM_DropIndex

```cpp
Four EduBtM_DropIndex(
    PhysicalFileID *pFid,	/* IN FileID of the Btree file */
    PageID *rootPid,		/* IN root PageID to be dropped */
    Pool   *dlPool,		/* INOUT pool of the dealloc list elements */
    DeallocListElem *dlHead) /* INOUT head of the dealloc list */
{
    Four e;			/* for the error number */

    // 색인 file에서 B+ tree 색인을 삭제함

    // 1) B+ tree 색인의 root page 및 모든 자식 page들을 각각 deallocate 함
    e = edubtm_FreePages(pFid, rootPid, dlPool, dlHead);
    if (e) ERR(e);
	
    return(eNOERROR);
    
} /* EduBtM_DropIndex() */
```

3. EduBtM_Fetch

```cpp
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
```

4. EduBtM_FetchNext

```cpp
Four EduBtM_FetchNext(
    PageID                      *root,          /* IN root page's PageID */
    KeyDesc                     *kdesc,         /* IN key descriptor */
    KeyValue                    *kval,          /* IN key value of stop condition */
    Four                        compOp,         /* IN comparison operator of stop condition */
    BtreeCursor                 *current,       /* IN current B+ tree cursor */
    BtreeCursor                 *next)          /* OUT next B+ tree cursor */
{
    int							i;
    Four                        e;              /* error number */
    Four                        cmp;            /* comparison result */
    Two                         slotNo;         /* slot no. of a leaf page */
    Two                         oidArrayElemNo; /* element no. of the array of ObjectIDs */
    Two                         alignedKlen;    /* aligned length of key length */
    PageID                      overflow;       /* temporary PageID of an overflow page */
    Boolean                     found;          /* search result */
    ObjectID                    *oidArray;      /* array of ObjectIDs */
    BtreeLeaf                   *apage;         /* pointer to a buffer holding a leaf page */
    BtreeOverflow               *opage;         /* pointer to a buffer holding an overflow page */
    btm_LeafEntry               *entry;         /* pointer to a leaf entry */
    BtreeCursor                 tCursor;        /* a temporary Btree cursor */
  
    
    /*@ check parameter */
    if (root == NULL || kdesc == NULL || kval == NULL || current == NULL || next == NULL)
	ERR(eBADPARAMETER_BTM);
    
    /* Is the current cursor valid? */
    if (current->flag != CURSOR_ON && current->flag != CURSOR_EOS)
		ERR(eBADCURSOR);
    
    if (current->flag == CURSOR_EOS) return(eNOERROR);
    
    /* Error check whether using not supported functionality by EduBtM */
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    // B+ tree 색인에서 검색 조건을 만족하는 현재 object의 다음 object를 검색하고, 검색된 object를 가리키는 cursor를 반환함

    // 1) edubtm_FetchNext()를 호출하여 B+ tree 색인에서 검색 조건을 만족하는 현재 leaf index entry의 다음 leaf index entry를 검색함
    // 2) 검색된 leaf index entry를 가리키는 cursor를 반환함
    
    e = edubtm_FetchNext(kdesc, kval, compOp, current, next);
	if (e) ERR(e);
    
    return(eNOERROR);
    
} /* EduBtM_FetchNext() */
```

5. EduBtM_InsertObject

```cpp
Four EduBtM_InsertObject(
    ObjectID *catObjForFile,	/* IN catalog object of B+ tree file */
    PageID   *root,		/* IN the root of Btree */
    KeyDesc  *kdesc,		/* IN key descriptor */
    KeyValue *kval,		/* IN key value */
    ObjectID *oid,		/* IN ObjectID which will be inserted */
    Pool     *dlPool,		/* INOUT pool of dealloc list */
    DeallocListElem *dlHead) /* INOUT head of the dealloc list */
{
    int i;
    Four e;			/* error number */
    Boolean lh;			/* for spliting */
    Boolean lf;			/* for merging */
    InternalItem item;		/* Internal Item */
    SlottedPage *catPage;	/* buffer page containing the catalog object */
    sm_CatOverlayForBtree *catEntry; /* pointer to Btree file catalog information */
    PhysicalFileID pFid;	 /* B+-tree file's FileID */

    
    /*@ check parameters */
    
    if (catObjForFile == NULL) ERR(eBADPARAMETER_BTM);
    
    if (root == NULL) ERR(eBADPARAMETER_BTM);

    if (kdesc == NULL) ERR(eBADPARAMETER_BTM);

    if (kval == NULL) ERR(eBADPARAMETER_BTM);

    if (oid == NULL) ERR(eBADPARAMETER_BTM);    

    /* Error check whether using not supported functionality by EduBtM */
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }
    
    // B+ tree 색인에 새로운 object를 삽입함

    // 1) edubtm_Insert()를 호출하여 새로운 object에 대한 <object의 key, object ID> pair를 B+ tree 색인에 삽입함
    edubtm_Insert(catObjForFile, root, kdesc, kval, oid, &lf, &lh, &item, dlPool, dlHead);
    // 2) Root page에서 split이 발생하여 새로운 root page 생성이 필요한 경우, edubtm_root_insert()를 호출하여 이를 처리함
    if(lh){
        edubtm_root_insert(catObjForFile, root, &item);
    }
    
    return(eNOERROR);
    
}   /* EduBtM_InsertObject() */
```

6. EduBtM_DeleteObject

```cpp
Four EduBtM_DeleteObject(
    ObjectID *catObjForFile,	/* IN catalog object of B+-tree file */
    PageID   *root,		/* IN root Page IDentifier */
    KeyDesc  *kdesc,		/* IN a key descriptor */
    KeyValue *kval,		/* IN key value */
    ObjectID *oid,		/* IN Object IDentifier */
    Pool     *dlPool,		/* INOUT pool of dealloc list elements */
    DeallocListElem *dlHead) /* INOUT head of the dealloc list */
{
    int		i;
    Four    e;			/* error number */
    Boolean lf;			/* flag for merging */
    Boolean lh;			/* flag for splitting */
    InternalItem item;		/* Internal item */
    SlottedPage *catPage;	/* buffer page containing the catalog object */
    sm_CatOverlayForBtree *catEntry; /* pointer to Btree file catalog information */
    PhysicalFileID pFid;        /* B+-tree file's FileID */


    /*@ check parameters */
    if (catObjForFile == NULL) ERR(eBADPARAMETER_BTM);

    if (root == NULL) ERR(eBADPARAMETER_BTM);

    if (kdesc == NULL) ERR(eBADPARAMETER_BTM);

    if (kval == NULL) ERR(eBADPARAMETER_BTM);

    if (oid == NULL) ERR(eBADPARAMETER_BTM);
    
    if (dlPool == NULL || dlHead == NULL) ERR(eBADPARAMETER_BTM);

    /* Error check whether using not supported functionality by EduBtM */
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    // B+ tree 색인에서 object를 삭제함

    MAKE_PAGEID(pFid, catObjForFile->volNo, catObjForFile->pageNo);
    // 1) edubtm_Delete()를 호출하여 삭제할 object에 대한 <object의 key, object ID> pair를 B+ tree 색인에서 삭제함
    edubtm_Delete(catObjForFile, root, kdesc, kval, oid, &lf, &lh, &item, dlPool, dlHead);
    if (lf == TRUE){
        // 2) Root page에서 underflow가 발생한 경우, btm_root_delete()를 호출하여 이를 처리함. 
        //     btm_root_delete()는 underflow가 발생한 root page가 비어있는지를 확인하여 비어있는 경우 root page를 삭제함
        btm_root_delete(&pFid, root, dlPool, dlHead);
        BfM_SetDirty(root, PAGE_BUF);
    }
    else if (lh == TRUE)
	{
        // 3) Root page에서 split이 발생한 경우, edubtm_root_insert()를 호출하여 이를 처리함
        //     Redistribute 과정 중에 root page의 index entry가 length가 더 긴 index entry로 교체되었을 수 있으므로, root page에서 split이 발생할 수 있음
		edubtm_root_insert(catObjForFile, root, &item);
        BfM_SetDirty(root, PAGE_BUF);
	}

	    
    return(eNOERROR);
    
}   /* EduBtM_DeleteObject() */
```

<Low Level>

1. edubtm_InitLeaf

```cpp
Four edubtm_InitLeaf(
    PageID *leaf,		/* IN the PageID to be initialized */
    Boolean root,		/* IN Is it root ? */
    Boolean isTmp)              /* IN Is it temporary ? */
{
    Four e;			/* error number */
    BtreeLeaf *page;		/* a page pointer */

    // 1) Page header를 leaf page로 초기화함
    BfM_GetNewTrain(leaf, (char**)&page, PAGE_BUF);

    page->hdr.pid = *leaf;
    page->hdr.flags = 3;
    if (root) {
        page->hdr.type = 0b101;
    } else {
        page->hdr.type = 0b100;
    }
    page->hdr.nSlots = 0;
    page->hdr.free = 0;
    page->hdr.prevPage = NIL;
    page->hdr.nextPage = NIL;
    page->hdr.unused = 0;

    e = BfM_SetDirty(leaf, PAGE_BUF);
    if(e) ERRB1(e, leaf, PAGE_BUF);
    e = BfM_FreeTrain(leaf, PAGE_BUF);
    if(e) ERR(e);
    
    return(eNOERROR);
    
}  /* edubtm_InitLeaf() */
```

2. edubtm_InitInternal

```cpp
Four edubtm_InitInternal(
    PageID  *internal,		/* IN the PageID to be initialized */
    Boolean root,		/* IN Is it root ? */
    Boolean isTmp)              /* IN Is it temporary ? - COOKIE12FEB98 */
{
    Four e;			/* error number */
    BtreeInternal *page;	/* a page pointer */

    // Page를 B+ tree 색인의 internal page로 초기화함

    // 1) Page header를 internal page로 초기화함
    BfM_GetNewTrain(internal, &page, PAGE_BUF);

    page->hdr.pid = *internal;
    page->hdr.flags = 5;
    page->hdr.type = INTERNAL;
    if(root)
        page->hdr.type |= ROOT;
    
    page->hdr.p0 = NIL;
    page->hdr.nSlots = 0;
    page->hdr.free = 0;
    page->hdr.unused = 0;

    BfM_SetDirty(internal, PAGE_BUF);
    BfM_FreeTrain(internal, PAGE_BUF);
    
    return(eNOERROR);
    
}  /* edubtm_InitInternal() */
```

3. edubtm_FreePages

```cpp
Four edubtm_FreePages(
    PhysicalFileID      *pFid,          /* IN FileID of the Btree file */
    PageID              *curPid,        /* IN The PageID to be freed */
    Pool                *dlPool,        /* INOUT pool of dealloc list elements */
    DeallocListElem     *dlHead)        /* INOUT head of the dealloc list */
{
    Four                e;              /* error number */
    Two                 i;              /* index */
    Two                 alignedKlen;    /* aligned length of the key length */
    PageID              tPid;           /* a temporary PageID */
    PageID              ovPid;          /* a temporary PageID of an overflow page */
    BtreePage           *apage;         /* a page pointer */
    BtreeOverflow       *opage;         /* page pointer to a buffer holding an overflow page */
    Two                 iEntryOffset;   /* starting offset of an internal entry */
    Two                 lEntryOffset;   /* starting offset of a leaf entry */
    btm_InternalEntry   *iEntry;        /* an internal entry */
    btm_LeafEntry       *lEntry;        /* a leaf entry */
    DeallocListElem     *dlElem;        /* an element of dealloc list */

    //B+ tree 색인 page를 deallocate 함

    //1) 파라미터로 주어진 page의 모든 자식 page들에 대해 재귀적으로 edubtm_FreePages()를 호출하여 해당 page들을 deallocate 함
    BfM_GetNewTrain(curPid, (char**)&apage, PAGE_BUF);

    if (apage->any.hdr.type & INTERNAL){
        BtreeInternal* curr = &(apage->bi);
        MAKE_PAGEID(tPid, curPid->volNo, curr->hdr.p0);
        edubtm_FreePages(pFid, &tPid, dlPool, dlHead);

        for (i = 0; i < curr->hdr.nSlots; i++) {
            iEntryOffset = apage->bi.slot[-i];
            iEntry = (btm_InternalEntry*)&(curr->data[iEntryOffset]);

            MAKE_PAGEID(tPid, curPid->volNo, iEntry->spid);
            edubtm_FreePages(pFid, &tPid, dlPool, dlHead);
        }
    }
    //2) 파라미터로 주어진 page를 deallocate 함
    //2-1) Page header의 type에서 해당 page가 deallocate 될 page임을 나타내는 bit를 set 및 나머지 bit들을 unset 함
    apage->any.hdr.type = FREEPAGE;
    //2-2) 해당 page를 deallocate 함
    Util_getElementFromPool(dlPool, &dlElem);
    dlElem->type = DL_PAGE;
    dlElem->elem.pid = *curPid;
    dlElem->next = dlHead->next;
    dlHead->next = dlElem;

    BfM_SetDirty(curPid, PAGE_BUF);
    BfM_FreeTrain(curPid, PAGE_BUF);
    
    return(eNOERROR);
    
}   /* edubtm_FreePages() */
```

4. edubtm_Fetch

```cpp
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
```

5. edubtm_FetchNext

```cpp
Four edubtm_FetchNext(
    KeyDesc  		*kdesc,		/* IN key descriptor */
    KeyValue 		*kval,		/* IN key value of stop condition */
    Four     		compOp,		/* IN comparison operator of stop condition */
    BtreeCursor 	*current,	/* IN current cursor */
    BtreeCursor 	*next)		/* OUT next cursor */
{
    Four 		e;		/* error number */
    Four 		cmp;		/* comparison result */
    Two 		alignedKlen;	/* aligned length of a key length */
    PageID 		leaf;		/* temporary PageID of a leaf page */
    PageID 		overflow;	/* temporary PageID of an overflow page */
    ObjectID 		*oidArray;	/* array of ObjectIDs */
    BtreeLeaf 		*apage;		/* pointer to a buffer holding a leaf page */
    BtreeOverflow 	*opage;		/* pointer to a buffer holding an overflow page */
    btm_LeafEntry 	*entry;		/* pointer to a leaf entry */    
    
    
    /* Error check whether using not supported functionality by EduBtM */
    int i;
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    // B+ tree 색인에서 검색 조건을 만족하는 현재 leaf index entry의 다음 leaf index entry를 검색하고, 검색된 leaf index entry를 가리키는 cursor를 반환함. 
    // 검색 조건이 SM_GT, SM_GE, SM_BOF일 경우 key 값이 작아지는 방향으로 backward scan을 하며, 그 외의 경우 key 값이 커지는 방향으로 forward scan을 한다

    // 1) 검색 조건을 만족하는 다음 leaf index entry를 검색함
    // 2) 검색된 leaf index entry를 가리키는 cursor를 반환함
    leaf = current->leaf;
	overflow = leaf;
	next->flag = CURSOR_ON;
    Two idx = current->slotNo;

    if(compOp == SM_EQ) next->flag = CURSOR_EOS;
    else if(compOp == SM_GT || compOp == SM_GE || compOp == SM_BOF) idx--; 
    else if(compOp == SM_LT || compOp == SM_LE || compOp == SM_EOF) idx++;

    BfM_GetTrain(&leaf, &apage, PAGE_BUF);

    if(next->flag != CURSOR_EOS) {
        if(idx < 0) {
            if(apage->hdr.prevPage != NIL) {
                BfM_FreeTrain(&leaf, PAGE_BUF);

                MAKE_PAGEID(overflow, leaf.volNo, apage->hdr.prevPage);
                BfM_GetTrain(&overflow, &apage, PAGE_BUF);

                idx = apage->hdr.nSlots - 1;
            }
            else {
                next->flag = CURSOR_EOS;
            }
        }
        else if(idx >= apage->hdr.nSlots) {
            if(apage->hdr.nextPage != NIL) {
                BfM_FreeTrain(&leaf, PAGE_BUF);

                MAKE_PAGEID(overflow, leaf.volNo, apage->hdr.nextPage);
                BfM_GetTrain(&overflow, &apage, PAGE_BUF);

                idx = 0;
            }
            else {
                next->flag = CURSOR_EOS;
            }
        }
        else {
            BfM_FreeTrain(&leaf, PAGE_BUF);
            BfM_GetTrain(&overflow, &apage, PAGE_BUF);
        }

        next->leaf = overflow;
		next->slotNo = idx;

        entry = &apage->data[apage->slot[-idx]];
		alignedKlen = ALIGNED_LENGTH(entry->klen);

		memcpy(&next->oid, &(entry->kval[alignedKlen]), OBJECTID_SIZE);
		memcpy(&next->key, &entry->klen, sizeof(KeyValue));

        cmp = edubtm_KeyCompare(kdesc, &next->key, kval);
		if ((compOp == SM_LE && (cmp != LESS && cmp != EQUAL))
            || (compOp == SM_LT && (cmp != LESS)) 
            || (compOp == SM_GE && (cmp != GREATER && cmp != EQUAL))
            || (compOp == SM_GT && (cmp != GREATER))){
                next->flag = CURSOR_EOS;
            }

        BfM_FreeTrain(&overflow, PAGE_BUF);
    }
    else{
        BfM_FreeTrain(&leaf, PAGE_BUF);
    }

    return(eNOERROR);
    
} /* edubtm_FetchNext() */
```

6. edubtm_FirstObject

```cpp
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
```

7. edubtm_LastObject

```cpp
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
    BfM_GetTrain(root, &apage, PAGE_BUF);

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
			lEntry = apage->bl.data + apage->bl.slot[-(apage->bl.hdr.nSlots-1)];
		
			memcpy(&cursor->key, &lEntry->klen, sizeof(KeyValue));

            cmp = edubtm_KeyCompare(kdesc, stopKval, &cursor->key);

            if(cmp == GREAT || (cmp == EQUAL && stopCompOp == SM_GT)){
                cursor->flag = CURSOR_EOS;
            }
            else {
                cursor->flag = CURSOR_ON;
                cursor->leaf = *root;
                cursor->slotNo = apage->bl.hdr.nSlots-1;
                cursor->oid = ((ObjectID *)&lEntry->kval[ALIGNED_LENGTH(cursor->key.len)])[0];
            }
		}
	}

	BfM_FreeTrain(root, PAGE_BUF);
    // 2) 검색 종료 key 값이 마지막 object의 key 값 보다 크거나, key 값은 같으나 검색 종료 연산이 SM_GT인 경우 CURSOR_EOS 반환 

    return(eNOERROR);
    
} /* edubtm_LastObject() */
```

8. edubtm_BinarySearchLeaf

```cpp
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
```

9. edubtm_BinarySearchInternal

```cpp
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
```

10. edubtm_KeyCompare

```cpp
Four edubtm_KeyCompare(
    KeyDesc                     *kdesc,		/* IN key descriptor for key1 and key2 */
    KeyValue                    *key1,		/* IN the first key value */
    KeyValue                    *key2)		/* IN the second key value */
{
    register unsigned char      *left;          /* left key value */
    register unsigned char      *right;         /* right key value */
    Two                         i;              /* index for # of key parts */
    Two                         j;              /* temporary variable */
    Two                         k;
    Two                         kpartSize;      /* size of the current kpart */
    Two                         len1, len2;	/* string length */
    Two_Invariable              s1, s2;         /* 2-byte short values */
    Four_Invariable             i1, i2;         /* 4-byte int values */
    Four_Invariable             l1, l2;         /* 4-byte long values */
    Eight_Invariable            ll1, ll2;       /* 8-byte long long values */
    float                       f1, f2;         /* float values */
    double                      d1, d2;		/* double values */
    PageID                      pid1, pid2;	/* PageID values */
    OID                         oid1, oid2;     /* OID values */
    

    /* Error check whether using not supported functionality by EduBtM */
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    // 파라미터로 주어진 두 key 값의 대소를 비교하고, 비교 결과를 반환함. Variable length string의 경우 사전식 순서를 이용하여 비교함

    // 1) 두 key 값이 같은 경우, EQUAL을 반환함
    // 2) 첫 번째 key 값이 큰 경우, GREATER를 반환함
    // 3) 첫 번째 key 값이 작은 경우, LESS를 반환함
    left = key1->val;
	right = key2->val;

    for(i = 0, j = 0, k = 0; i < kdesc->nparts; i++) {
        switch(kdesc->kpart[i].type) {
        case SM_INT:
            kpartSize = kdesc->kpart[i].length;
			memcpy(&i1, &left[j], kpartSize);
			memcpy(&i2, &right[k], kpartSize);

            if(i1 < i2) return LESS;
            else if(i1 > i2) return GREAT;

            j += kpartSize;
			k += kpartSize;

            break;

        case SM_VARSTRING:
            memcpy(&len1, &left[j], sizeof(Two));
	        memcpy(&len2, &right[k], sizeof(Two));

            if (strcmp(&left[j+sizeof(Two)], &right[j+sizeof(Two)]) > 0)
				return (GREAT);
			else if (strcmp(&left[j+sizeof(Two)], &right[j+sizeof(Two)]) < 0)
				return (LESS);
			
			j += len1 + sizeof(Two);
			k += len2 + sizeof(Two);
			
            break;
        default:
            break;
        }
    }
        
    return(EQUAL);
    
}   /* edubtm_KeyCompare() */
```

11. edubtm_Insert

```cpp
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

    BfM_GetTrain(root, &apage, PAGE_BUF);

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
            tKey.len = litem.klen;
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
```

12. edubtm_root_insert

```cpp
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
```

13. edubtm_InsertInternal

```cpp
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
```

14. edubtm_InsertLeaf

```cpp
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
```

15. edubtm_SplitLeaf

```cpp
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
    edubtm_InitLeaf(&newPid, FALSE, FALSE);
    // 3) 기존 index entry들 및 삽입할 index entry를 key 순으로 정렬하여 overflow가 발생한 page 및 할당 받은 page에 나누어 저장함
    BfM_GetNewTrain(&newPid, (char**)&npage, PAGE_BUF);
    // 먼저, overflow가 발생한 page에 데이터 영역을 50% 이상 채우는 수의 index entry들을 저장하고 header 갱신
    // 나머지 index entry들을 할당 받은 page에 저장하고 header 갱신
    sum = 0;
    maxLoop = fpage->hdr.nSlots + 1;
    flag = FALSE;
    for(i = 0, j = 0; i < maxLoop && sum < BL_HALF; i++){
        if(i == high + 1){
            entryLen = (sizeof(Two) * 2 + ALIGNED_LENGTH(item->klen) + OBJECTID_SIZE); 

            flag = TRUE;
        }
        else{
            fEntryOffset = fpage->slot[-j];
            fEntry = &fpage->data[fEntryOffset];
            entryLen = (sizeof(Two) * 2 + ALIGNED_LENGTH(fEntry->klen) + OBJECTID_SIZE); 
            j++;
        }
        sum += entryLen + sizeof(Two);

    }
    fpage->hdr.nSlots = j;

    for(k = 0; i < maxLoop; i++){
        nEntryOffset = npage->hdr.free;
        npage->slot[-k] = nEntryOffset; 
        nEntry = &npage->data[nEntryOffset];

        if(!flag && (i == high + 1)){
            nEntry->nObjects = item->nObjects;
            nEntry->klen = item->klen;

            memcpy(nEntry->kval, item->kval, item->klen);
            memcpy(&nEntry->kval[ALIGNED_LENGTH(item->klen)], &item->oid, OBJECTID_SIZE);

            entryLen = (sizeof(Two) * 2 + ALIGNED_LENGTH(item->klen) + OBJECTID_SIZE);
        }
        else{
            fEntryOffset = fpage->slot[-j];
            fEntry = &fpage->data[fEntryOffset];
            entryLen = (sizeof(Two) * 2 + ALIGNED_LENGTH(fEntry->klen) + OBJECTID_SIZE); 
            
            memcpy(nEntry, fEntry, entryLen);

            if(fEntryOffset + entryLen == fpage->hdr.free){
                fpage->hdr.free -= entryLen;
            }
            else{
                fpage->hdr.unused += entryLen;  
            }

            j++;
        }

        npage->hdr.free += entryLen;
        k++;
    }
    npage->hdr.nSlots = k;

    if(flag == TRUE){
        for(i = fpage->hdr.nSlots - 1; i > high; i--){
            fpage->slot[-(i+1)] = fpage->slot[-i];
        }
        fpage->slot[-(high+1)] = fpage->hdr.free;

        fEntry = &fpage->data[fpage->hdr.free];
        fEntry->nObjects = item->nObjects;
        fEntry->klen = item->klen;

        memcpy(fEntry->kval, item->kval, item->klen);
        memcpy(&fEntry->kval[ALIGNED_LENGTH(item->klen)], &item->oid, OBJECTID_SIZE);

        fpage->hdr.free += (sizeof(Two) * 2 + ALIGNED_LENGTH(item->klen) + OBJECTID_SIZE);
        fpage->hdr.nSlots ++;
    }
    
    // 4) 할당 받은 page를 leaf page들간의 doubly linked list에 추가함
    npage->hdr.prevPage = root->pageNo;
    npage->hdr.nextPage = fpage->hdr.nextPage;
    fpage->hdr.prevPage;
    fpage->hdr.nextPage = newPid.pageNo;

    if(npage->hdr.nextPage != NIL){
        nextPid.pageNo = npage->hdr.nextPage;
        nextPid.volNo = npage->hdr.pid.volNo;

        BfM_GetTrain(&nextPid, &mpage, PAGE_BUF);
        mpage->hdr.prevPage = newPid.pageNo;
        BfM_SetDirty(&nextPid, PAGE_BUF);
        BfM_FreeTrain(&nextPid, PAGE_BUF);
    }

    // 5) 할당 받은 page를 가리키는 internal index entry를 생성함
    nEntry = &npage->data[npage->slot[0]];
    ritem->spid = newPid.pageNo;
    ritem->klen = nEntry->klen;
    memcpy(ritem->kval, nEntry->kval, nEntry->klen);

    // 6) Split된 page가 ROOT일 경우, type을 LEAF로 변경함
    if(fpage->hdr.type & ROOT){
        fpage->hdr.type = LEAF;
    }
    // 7) 생성된 index entry를 반환함
    BfM_SetDirty(&newPid, PAGE_BUF);
    BfM_FreeTrain(&newPid, PAGE_BUF);

    return(eNOERROR);
    
} /* edubtm_SplitLeaf() */
```

16. edubtm_SplitInternal

```cpp
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

    btm_InternalEntry           *itemEntry;
    Two                         itemEntryLen;

    // Overflow가 발생한 internal page를 split 하여 파라미터로 주어진 index entry를 삽입하고, split으로 생성된 새로운 internal page를 가리키는 internal index entry를 반환함

    // 1) 새로운 page를 할당 받음
    btm_AllocPage(catObjForFile, &fpage->hdr.pid, &newPid);
    // 2) 할당 받은 page를 internal page로 초기화함
    edubtm_InitInternal(&newPid, FALSE, FALSE);
    // 3) 기존 index entry들 및 삽입할 index entry를 key 순으로 정렬하여 overflow가 발생한 page 및 할당 받은 page에 나누어 저장함
    BfM_GetNewTrain(&newPid, &npage, PAGE_BUF);
    // 3-1) 먼저, overflow가 발생한 page에 데이터 영역을 50% 이상 채우는 수의 index entry들을 저장함
    // 3-2) 할당 받은 page의 header의 p0 변수에 아직 저장되지 않은 index entry 중 첫 번째 index entry (1st entry) 가 가리키는 자식 page의 번호를 저장함
    // 3-3) 1st entry는 할당 받은 page를 가리키는 internal index entry로 설정하여 반환함 (자식 page의 번호 := 할당 받은 page의 번호)
    // 3-4) 나머지 index entry들을 할당 받은 page에 저장함
    // 3-5) 각 page의 header를 갱신함
    sum = 0;
    maxLoop = fpage->hdr.nSlots + 1;
    flag = FALSE;
    for(i = 0, j = 0; i < maxLoop && sum < BI_HALF; i++){
        if(i == high + 1){
            entryLen = sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(item->klen); 
            flag = TRUE;
        }
        else{
            fEntryOffset = fpage->slot[-j];
            fEntry = &fpage->data[fEntryOffset];
            entryLen = sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(fEntry->klen); 
            j++;
        }
        sum += entryLen + sizeof(Two);
    }
    fpage->hdr.nSlots = j;

    for(k = -1; i < maxLoop; i++){
        nEntryOffset = npage->hdr.free;
        if(k != -1){
            npage->slot[-k] = nEntryOffset; 
        }
        nEntry = &npage->data[nEntryOffset];

        if(!flag && (i == high + 1)){
            if(k == -1){
                memcpy(ritem, item, sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(item->klen));
            }
            else{
                memcpy(nEntry, item, sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(item->klen));
            }
            entryLen = sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(item->klen);
        }
        else{
            fEntryOffset = fpage->slot[-j];
            fEntry = &fpage->data[fEntryOffset];
            entryLen = (sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(fEntry->klen)); 

            if(k == -1){
                memcpy(ritem, fEntry, entryLen);
            }
            else{
                memcpy(nEntry, fEntry, entryLen);
            }

            if(fEntryOffset + entryLen == fpage->hdr.free){
                fpage->hdr.free -= entryLen;
            }
            else{
                fpage->hdr.unused += entryLen;
            }

            j++;
        }

        if(k == -1){
            npage->hdr.p0 = ritem->spid;
        }
        else{
            npage->hdr.free += entryLen;
        }
        k++;
    }
    npage->hdr.nSlots = k;

    if(flag == TRUE){
        for(i = fpage->hdr.nSlots - 1; i > high; i--){
            fpage->slot[-(i+1)] = fpage->slot[-i];
        }
        fpage->slot[-(high+1)] = fpage->hdr.free;

        fEntry = &fpage->data[fpage->hdr.free];

        memcpy(fEntry, item, sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(item->klen));

        fpage->hdr.free += sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(item->klen);
        fpage->hdr.nSlots++;
    }

    // 4) Split된 page가 ROOT일 경우, type을 INTERNAL로 변경함
    if(fpage->hdr.type & ROOT){
        fpage->hdr.type = INTERNAL;
    }

    BfM_SetDirty(&newPid, PAGE_BUF);
    BfM_FreeTrain(&newPid, PAGE_BUF);
    
    return(eNOERROR);
    
} /* edubtm_SplitInternal() */
```

17. edubtm_CompactLeafPage

```cpp
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

			memcpy(tpage.data + apageDataOffset, entry, 2 * sizeof(Two) + ALIGNED_LENGTH(entry->klen) + OBJECTID_SIZE);
			apage->slot[-i] = apageDataOffset;
			apageDataOffset += 2 * sizeof(Two) + ALIGNED_LENGTH(entry->klen) + OBJECTID_SIZE;
		}
	}
    if (slotNo != NIL)
	{
		entry = apage->data + apage->slot[-slotNo];
        
		memcpy(tpage.data + apageDataOffset, entry, 2 * sizeof(Two) + ALIGNED_LENGTH(entry->klen) + OBJECTID_SIZE);
		apage->slot[-slotNo] = apageDataOffset;
		apageDataOffset += 2 * sizeof(Two) + ALIGNED_LENGTH(entry->klen) + OBJECTID_SIZE;
	}
    memcpy(apage->data, tpage.data, apageDataOffset);
    // 3) Page header를 갱신함
    apage->hdr.free = apageDataOffset;
	apage->hdr.unused = 0;

} /* edubtm_CompactLeafPage() */
```

18. edubtm_CompactInternalPage

```cpp
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
            
            memcpy(tpage.data + apageDataOffset, entry, sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(entry->klen));
            apage->slot[-i] = apageDataOffset;
            apageDataOffset += sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(entry->klen);
        }
    }
    if(slotNo != NIL){
        entry = apage->data + apage->slot[-slotNo];

        memcpy(tpage.data + apageDataOffset, entry, sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(entry->klen));
        apage->slot[-slotNo] = apageDataOffset;
        apageDataOffset += sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(entry->klen);
    }
    memcpy(apage->data, tpage.data, apageDataOffset);
    // 3) Page header를 갱신함
    apage->hdr.free = apageDataOffset;
    apage->hdr.unused = 0;

} /* edubtm_CompactInternalPage() */
```

19. edubtm_Delete

```cpp
Four edubtm_Delete(
    ObjectID                    *catObjForFile, /* IN catalog object of B+ tree file */
    PageID                      *root,          /* IN root page */
    KeyDesc                     *kdesc,         /* IN a key descriptor */
    KeyValue                    *kval,          /* IN key value */
    ObjectID                    *oid,           /* IN Object IDentifier which will be deleted */
    Boolean                     *f,             /* OUT whether the root page is half full */
    Boolean                     *h,             /* OUT TRUE if it is spiltted. */
    InternalItem                *item,          /* OUT The internal item to be returned */
    Pool                        *dlPool,        /* INOUT pool of dealloc list elements */
    DeallocListElem             *dlHead)        /* INOUT head of the dealloc list */
{
    Four                        e;              /* error number */
    Boolean                     lf;             /* TRUE if a page is not half full */
    Boolean                     lh;             /* TRUE if a page is splitted */
    Two                         idx;            /* the index by the binary search */
    PageID                      child;          /* a child page when the root is an internal page */
    KeyValue                    tKey;           /* a temporary key */
    BtreePage                   *rpage;         /* for a root page */
    InternalItem                litem;          /* local internal item */
    btm_InternalEntry           *iEntry;        /* an internal entry */
    SlottedPage                 *catPage;       /* buffer page containing the catalog object */
    sm_CatOverlayForBtree       *catEntry;      /* pointer to Btree file catalog information */
    PhysicalFileID              pFid;           /* B+-tree file's FileID */
  

    /* Error check whether using not supported functionality by EduBtM */
   int i;
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }
        
    *h = *f = FALSE;

    // 파라미터로 주어진 page를 root page로 하는 B+ tree 색인에서 <object의 key, object ID> pair를 삭제함

    BfM_GetTrain(catObjForFile, &catPage, PAGE_BUF);
    GET_PTR_TO_CATENTRY_FOR_BTREE(catObjForFile, catPage, catEntry);
    MAKE_PAGEID(pFid, catEntry->fid.volNo, catEntry->firstPage);
    BfM_GetTrain(root, &rpage, PAGE_BUF);
    if((rpage->any.hdr.type) & INTERNAL){
        // 1) 파라미터로 주어진 root page가 internal page인 경우
    
        // 1-1) 삭제할 <object의 key, object ID> pair가 저장된 leaf page를 찾기 위해 다음으로 방문할 자식 page를 결정함
        edubtm_BinarySearchInternal(&rpage->bi, kdesc, kval, &idx);
        if(idx < 0){
            MAKE_PAGEID(child, root->volNo, rpage->bi.hdr.p0);
        }
        else{
            iEntry = &rpage->bi.data[rpage->bi.slot[-idx]];
            MAKE_PAGEID(child, root->volNo, iEntry->spid);
        }
        // 1-2) 결정된 자식 page를 root page로 하는 B+ subtree에서 <object의 key, object ID> pair를 삭제하기 위해 재귀적으로 edubtm_Delete()를 호출함
        edubtm_Delete(catObjForFile, &child, kdesc, kval, oid, &lf, &lh, &litem, dlPool, dlHead);
        // 1-3) 결정된 자식 page에서 underflow가 발생한 경우, btm_Underflow()를 호출하여 이를 처리함
        //     Underflow가 발생한 자식 page의 부모 page (파라미터로 주어진 root page) 에서 overflow가 발생한 경우, 
        //     edubtm_InsertInternal()을 호출하여 overflow로 인해 삽입되지 못한 internal index entry를 부모 page에 삽입함. 
        //     edubtm_InsertInternal() 호출 결과로서 부모 page가 split 되므로, out parameter인 h를 TRUE로 설정하고 
        //     split으로 생성된 새로운 page를 가리키는 internal index entry를 반환함
        //     btm_Underflow() 호출 결과로서 파라미터로 주어진 root page의 내용이 변경되므로, btm_Underflow() 호출 후 root page의 DIRTY bit를 1로 set 해야 함
        if(lf){
            btm_Underflow(&pFid, rpage, &child, idx, f, &lh, &litem, dlPool, dlHead);
            BfM_SetDirty(root, PAGE_BUF);
            if(lh){
                memcpy(&tKey, &litem.klen, sizeof(KeyValue));
                edubtm_BinarySearchInternal(rpage, kdesc, &tKey, &idx);
                edubtm_InsertInternal(catObjForFile, rpage, &litem, idx, h, item);
            }
        }
    }
    else if((rpage->any.hdr.type) & LEAF){
        // 2) 파라미터로 주어진 root page가 leaf page인 경우,
        // 2-1) edubtm_DeleteLeaf()를 호출하여 해당 page에서 <object의 key, object ID> pair를 삭제함
        // 2-2) 해당 page에서 underflow가 발생한 경우 (page의 data 영역 중 자유 영역의 크기 > (page의 data 영역의 전체 크기 / 2)), out parameter인 f를 TRUE로 설정함
        edubtm_DeleteLeaf(&pFid, root, rpage, kdesc, kval, oid, f, h, item, dlPool, dlHead);
    }
    
    BfM_FreeTrain(catObjForFile, PAGE_BUF);
    BfM_FreeTrain(root, PAGE_BUF);
   
    return(eNOERROR);
    
}   /* edubtm_Delete() */
```

20. edubtm_DeleteLeaf

```cpp
Four edubtm_DeleteLeaf(
    PhysicalFileID              *pFid,          /* IN FileID of the Btree file */
    PageID                      *pid,           /* IN PageID of the leaf page */
    BtreeLeaf                   *apage,         /* INOUT buffer for the Leaf Page */
    KeyDesc                     *kdesc,         /* IN a key descriptor */
    KeyValue                    *kval,          /* IN key value */
    ObjectID                    *oid,           /* IN ObjectID which will be deleted */
    Boolean                     *f,             /* OUT whether the root page is half full */
    Boolean                     *h,             /* OUT TRUE if it is spiltted. */
    InternalItem                *item,          /* OUT The internal item to be returned */
    Pool                        *dlPool,        /* INOUT pool of dealloc list elements */
    DeallocListElem             *dlHead)        /* INOUT head of a dealloc list */
{
    Four                        e;              /* error number */
    Two                         i;              /* index */
    Two                         of;             /* # of ObjectIDs of an overflow page when less than 1/4 */
    Two                         idx;            /* the index by the binary search */
    ObjectID                    tOid;           /* a Object IDentifier */
    BtreeOverflow               *opage;         /* for a overflow page */
    Boolean                     found;          /* Search Result */
    Two                         lEntryOffset;   /* starting offset of a leaf entry */
    btm_LeafEntry               *lEntry;        /* an entry in leaf page */
    ObjectID                    *oidArray;      /* start position of the ObjectID array */
    Two                         oidArrayElemNo; /* element number in the ObjectIDs array */
    Two                         entryLen;       /* length of the old leaf entry */
    Two                         newLen;         /* length of the new leaf entry */
    Two                         alignedKlen;    /* aligned length of the key length */
    PageID                      ovPid;          /* overflow page's PageID */
    DeallocListElem             *dlElem;        /* an element of the dealloc list */


    /* Error check whether using not supported functionality by EduBtM */
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    // Leaf page에서 <object의 key, object ID> pair를 삭제함

    // 1) 삭제할 <object의 key, object ID> pair가 저장된 index entry의 offset이 저장된 slot을 삭제함
    //     Slot array 중간에 삭제된 빈 slot이 없도록 slot array를 compact 함
    // 2) Leaf page의 header를 갱신함
    found = edubtm_BinarySearchLeaf(apage, kdesc, kval, &idx);
    if(found){
        lEntry = &apage->data[apage->slot[-idx]];
        lEntryOffset = apage->slot[-idx];
        entryLen = sizeof(Two) * 2 + ALIGNED_LENGTH(lEntry->klen) + OBJECTID_SIZE;

        memcpy(&tOid, &lEntry->kval[ALIGNED_LENGTH(lEntry->klen)], OBJECTID_SIZE);

        if(btm_ObjectIdComp(oid, &tOid) == EQUAL){
            for(i = idx; i < apage->hdr.nSlots - 1; i++){
                apage->slot[-i] = apage->slot[-(i+1)];
            }

            if(lEntryOffset + entryLen == apage->hdr.free){
                apage->hdr.free -= entryLen;
            }
            else{
                apage->hdr.unused += entryLen;
            }
            apage->hdr.nSlots--;
        }
    }
    // 3) Leaf page에서 underflow가 발생한 경우(page의 data 영역 중 자유 영역의 크기 > (page의 data 영역의 전체 크기 / 2)), out parameter인 f를 TRUE로 설정함
    if(BL_FREE(apage) > BL_HALF){
        *f = TRUE;
    }
    BfM_SetDirty(pid, PAGE_BUF);
         
    return(eNOERROR);
    
} /* edubtm_DeleteLeaf() */
```