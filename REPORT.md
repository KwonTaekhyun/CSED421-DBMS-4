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

FILL WITH YOUR CODE IMPLEMENTATION DESCRIPTION