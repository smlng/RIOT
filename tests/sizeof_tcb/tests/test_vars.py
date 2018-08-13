REGEXP_TCB_TOTAL = r'sizeof\(thread_t\):\s+(?P<size>\d+)'
REGEXP_TCB_PARTS = r'\s+\w+\s+(?P<size>\d+)\s+\d+'

LIST__TCB_PARTS_32 = [
    '\tsp            4   0',
    '\tstatus        1   4',
    '\tpriority      1   5',
    '\tpid           2   6',
    '\trq_entry      4   8',
    '\twait_data     4  12',
    '\tmsg_waiters   4  16',
    '\tmsg_queue    12  20',
    '\tmsg_array     4  32',
]
LIST__TCB_PARTS_16 = [
    '\tsp            2   0',
    '\tstatus        1   2',
    '\tpriority      1   3',
    '\tpid           2   4',
    '\trq_entry      2   6',
    '\twait_data     2   8',
    '\tmsg_waiters   2  10',
    '\tmsg_queue     6  12',
    '\tmsg_array     2  18',
]
