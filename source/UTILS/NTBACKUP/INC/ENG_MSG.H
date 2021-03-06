#ifndef ENG_MSG_SH

#define ENG_MSG_SH

#define RES_MESSAGE                      3000

#define RES_YES_KEY                      RES_MESSAGE +    0
#define RES_NO_KEY                       RES_MESSAGE +    1
#define RES_YES_STRING                   RES_MESSAGE +    2
#define RES_NO_STRING                    RES_MESSAGE +    3
#define RES_PRE_NOON                     RES_MESSAGE +    4
#define RES_AFTER_NOON                   RES_MESSAGE +    5
#define RES_SAVED_CONFIG                 RES_MESSAGE +    6
#define RES_BACKUP_STATUS_LABEL          RES_MESSAGE +    7
#define RES_ARCHIVE_STATUS_LABEL         RES_MESSAGE +    8
#define RES_RESTORE_STATUS_LABEL         RES_MESSAGE +    9
#define RES_VERIFY_STATUS_LABEL          RES_MESSAGE +    10
#define RES_DIRECTORY_STATUS_LABEL       RES_MESSAGE +    11
#define RES_INSERT_NEW_TAPE              RES_MESSAGE +    12
#define RES_REPLACE_OLD_TAPE             RES_MESSAGE +    13
#define RES_CONTINUE                     RES_MESSAGE +    14
#define RES_TAPE_NOT_INSERTED            RES_MESSAGE +    15
#define RES_DISPLAY_VCB                  RES_MESSAGE +    16
#define RES_APPEND_QUEST                 RES_MESSAGE +    17
#define RES_REPLACE_WARNING              RES_MESSAGE +    18
#define RES_REPLACE_TAPE                 RES_MESSAGE +    19
#define RES_RESTORE_QUEST                RES_MESSAGE +    20
#define RES_VERIFY_QUEST                 RES_MESSAGE +    21
#define RES_DIRECTORY_QUEST              RES_MESSAGE +    22
#define RES_SEARCH_QUEST                 RES_MESSAGE +    23
#define RES_CONTINUE_QUEST               RES_MESSAGE +    24
#define RES_FOREIGN_TAPE_MSG             RES_MESSAGE +    25
#define RES_BLANK_TAPE                   RES_MESSAGE +    26
#define RES_NO_MORE_TAPE_INFO            RES_MESSAGE +    27
#define RES_SEARCHING                    RES_MESSAGE +    28
#define RES_REWINDING                    RES_MESSAGE +    29
#define RES_WAITING                      RES_MESSAGE +    30
#define RES_BACKUP_STARTED               RES_MESSAGE +    31
#define RES_RESTORE_STARTED              RES_MESSAGE +    32
#define RES_VERIFY_STARTED               RES_MESSAGE +    33
#define RES_DIRECTORY                    RES_MESSAGE +    34
#define RES_FILE_SKIPPED                 RES_MESSAGE +    35
#define RES_BACKUP_COMPLETED             RES_MESSAGE +    36
#define RES_RESTORE_COMPLETED            RES_MESSAGE +    37
#define RES_VERIFY_COMPLETED             RES_MESSAGE +    38
#define RES_BACKED_UP_DIR_FILE           RES_MESSAGE +    39
#define RES_BACKED_UP_DIR_FILES          RES_MESSAGE +    40
#define RES_BACKED_UP_DIRS_FILES         RES_MESSAGE +    41
#define RES_BACKED_UP_MAC                RES_MESSAGE +    42
#define RES_BACKED_UP_MACS               RES_MESSAGE +    43
#define RES_BACKED_UP_CORRUPT_WARNING    RES_MESSAGE +    44
#define RES_BACKED_UP_CORRUPT            RES_MESSAGE +    45
#define RES_BACKED_UP_CORRUPTS           RES_MESSAGE +    46
#define RES_BACKED_UP_IN_USE_WARNING     RES_MESSAGE +    47
#define RES_BACKED_UP_IN_USE             RES_MESSAGE +    48
#define RES_BACKED_UP_IN_USES            RES_MESSAGE +    49
#define RES_FILE_SKIPPED_STAT            RES_MESSAGE +    50
#define RES_FILE_SKIPPEDS_STAT           RES_MESSAGE +    51
#define RES_RESTORED_DIR_FILE            RES_MESSAGE +    52
#define RES_RESTORED_DIR_FILES           RES_MESSAGE +    53
#define RES_RESTORED_DIRS_FILES          RES_MESSAGE +    54
#define RES_RESTORED_MAC                 RES_MESSAGE +    55
#define RES_RESTORED_MACS                RES_MESSAGE +    56
#define RES_RESTORED_CORRUPT             RES_MESSAGE +    57
#define RES_RESTORED_CORRUPTS            RES_MESSAGE +    58
#define RES_RESTORED_CORRUPT_WARNING     RES_MESSAGE +    59
#define RES_RESTORED_IN_USE              RES_MESSAGE +    60
#define RES_RESTORED_IN_USES             RES_MESSAGE +    61
#define RES_RESTORED_IN_USE_WARNING      RES_MESSAGE +    62
#define RES_VERIFIED_DIR_FILE            RES_MESSAGE +    63
#define RES_VERIFIED_DIR_FILES           RES_MESSAGE +    64
#define RES_VERIFIED_DIRS_FILES          RES_MESSAGE +    65
#define RES_VERIFIED_MAC                 RES_MESSAGE +    66
#define RES_VERIFIED_MACS                RES_MESSAGE +    67
#define RES_FILE_IS_DIFFERENT            RES_MESSAGE +    68
#define RES_FILES_DIFFERENT              RES_MESSAGE +    69
#define RES_FILE_NOT_FOUND_ON_DISK       RES_MESSAGE +    70
#define RES_PROCESSED_BYTES              RES_MESSAGE +    71
#define RES_PROCESSED_BYTES_MIN          RES_MESSAGE +    72
#define RES_PROCESSED_BYTES_SEC          RES_MESSAGE +    73
#define RES_PROCESS_RATE                 RES_MESSAGE +    74
#define RES_VERIFIED_DIRS_FILE           RES_MESSAGE +    75
#define RES_RESTORE_TITLE                RES_MESSAGE +    76           /* open for use */
#define RES_TENSION_TITLE                RES_MESSAGE +    77           /* open for use */
#define RES_ERASE_TITLE                  RES_MESSAGE +    78           /* open for use */
#define RES_FILE_READ_ERROR              RES_MESSAGE +    79
#define RES_FILE_WRITE_ERROR             RES_MESSAGE +    80
#define RES_DIR_STARTED                  RES_MESSAGE +    81
#define RES_DIR_COMPLETED                RES_MESSAGE +    82
#define RES_DIR_DIR_FILE                 RES_MESSAGE +    83
#define RES_DIR_DIR_FILES                RES_MESSAGE +    84
#define RES_DIR_DIRS_FILES               RES_MESSAGE +    85
#define RES_DIR_MAC                      RES_MESSAGE +    86
#define RES_DIR_MACS                     RES_MESSAGE +    87
#define RES_DIR_CORRUPT_WARNING          RES_MESSAGE +    88
#define RES_DIR_CORRUPT                  RES_MESSAGE +    89
#define RES_DIR_CORRUPTS                 RES_MESSAGE +    90
#define RES_DIR_IN_USE_WARNING           RES_MESSAGE +    91
#define RES_DIR_IN_USE                   RES_MESSAGE +    92
#define RES_DIR_IN_USES                  RES_MESSAGE +    93
#define RES_WRITE_PROTECTED              RES_MESSAGE +    94
#define RES_UNKNOWN_DEVICE               RES_MESSAGE +    95
#define RES_DRIVE_NOT_READY              RES_MESSAGE +    96
#define RES_SEEK_ERROR                   RES_MESSAGE +    97
#define RES_SECTOR_NOT_FOUND             RES_MESSAGE +    98
#define RES_PRINTER_ERROR                RES_MESSAGE +    99
#define RES_WRITE_ERROR                  RES_MESSAGE +    100
#define RES_READ_ERROR                   RES_MESSAGE +    101
#define RES_GENERAL_FAILURE              RES_MESSAGE +    102
#define RES_UNRECOVERABLE_DISK_ERROR     RES_MESSAGE +    103
#define RES_SKIPPED_SCRIPT_HEADER        RES_MESSAGE +    104
#define RES_SKIPPED_SCRIPT_ERROR         RES_MESSAGE +    105
#define RES_WAITING_FOR_OPEN             RES_MESSAGE +    106
#define RES_DISPLAY_VOLUME               RES_MESSAGE +    107
#define RES_CORRUPT_HEADER               RES_MESSAGE +    108
#define RES_CATALOG_MAINTENANCE_STATUS   RES_MESSAGE +    109
#define RES_CATALOG_DIRECTORY_STATUS     RES_MESSAGE +    110
#define RES_CATALOG_TAPE_SUCCESS_MSG     RES_MESSAGE +    111
#define RES_CATALOG_TAPE_ERROR_MSG       RES_MESSAGE +    112
#define RES_CATALOG_CONTINUATION_MSG     RES_MESSAGE +    113
#define RES_INSERT_CONTINUATION_MSG      RES_MESSAGE +    114
#define RES_INSERT_TAPE_TO_CATALOG_MSG   RES_MESSAGE +    115
#define RES_CAT_DELETE_SUCCESS_MSG       RES_MESSAGE +    116
#define RES_CAT_NO_ITEMS_MSG             RES_MESSAGE +    117
#define RES_FILE_DIFFERENT               RES_MESSAGE +    118
#define RES_ON_TAPE                      RES_MESSAGE +    119
#define RES_ON_DISK                      RES_MESSAGE +    120
#define RES_VERIFY_SCRIPT_HEADER         RES_MESSAGE +    121
#define RES_VERIFY_DATA_DIFFERENCE       RES_MESSAGE +    122
#define RES_VERIFY_OPEN_ERROR            RES_MESSAGE +    123
#define RES_VERIFY_DATA_VERIFIED         RES_MESSAGE +    124
#define RES_SKIPPING_REMOTE              RES_MESSAGE +    125
#define RES_CANT_SEARCH                  RES_MESSAGE +    126
#define RES_DELETE_STARTED               RES_MESSAGE +    127
#define RES_DELETE_COMPLETED             RES_MESSAGE +    128
#define RES_DELETE_DIR_FILE              RES_MESSAGE +    129
#define RES_DELETE_DIR_FILES             RES_MESSAGE +    130
#define RES_DELETE_DIRS_FILES            RES_MESSAGE +    131
#define RES_DELETE_MAC                   RES_MESSAGE +    132
#define RES_DELETE_MACS                  RES_MESSAGE +    133
#define RES_RESTORE_FILE_PROMPT          RES_MESSAGE +    134
#define RES_ENTER_TAPE_PASSWORD          RES_MESSAGE +    135
#define RES_NO_TAPE_PASSWORD             RES_MESSAGE +    136
#define RES_CONFIRM_TAPE_PASSWORD        RES_MESSAGE +    137
#define RES_TAPE_PASSWORD_MISMATCH       RES_MESSAGE +    138
#define RES_TAPE_PASSWORD_MATCH          RES_MESSAGE +    139
#define RES_VERIFY_TAPE_PASSWORD         RES_MESSAGE +    140
#define RES_VERIFY_PASSWORD_MISMATCH     RES_MESSAGE +    141
#define RES_NO_PREVIOUS_MATCH            RES_MESSAGE +    142
#define RES_NO_NEXT_MATCH                RES_MESSAGE +    143
#define RES_TAPE_CREATED                 RES_MESSAGE +    144
#define RES_TAPE_REQUEST                 RES_MESSAGE +    145
#define RES_TAPE_FAMILY_CONTINUES_MSG    RES_MESSAGE +    146
#define RES_DIR_EMPTY_MSG                RES_MESSAGE +    147
#define RES_DIR_SUCCESS_MSG              RES_MESSAGE +    148
#define RES_CATALOG_A_TAPE_STATUS        RES_MESSAGE +    149
#define RES_FILE_NOT_FOUND               RES_MESSAGE +    150
#define RES_FILES_NOT_FOUND              RES_MESSAGE +    151
#define RES_DIRECTORY_NOT_FOUND          RES_MESSAGE +    152
#define RES_DIRECTORYS_NOT_FOUND         RES_MESSAGE +    153
#define RES_DIRECTORY_NOT_FOUND_ON_DISK  RES_MESSAGE +    154
#define RES_FULL_TAPE                    RES_MESSAGE +    155
#define RES_TAPE_WEAR                    RES_MESSAGE +    156
#define RES_INSERT_NEXT_TAPE             RES_MESSAGE +    157
#define RES_WRONG_TAPE                   RES_MESSAGE +    158
#define RES_ARCHIVE_REPLACE_WARNING      RES_MESSAGE +    159
#define RES_CORRUPT_RESTORE_WARNING      RES_MESSAGE +    160
#define RES_OUT_OF_SEQUENCE_WARNING      RES_MESSAGE +    161
#define RES_RESTORE_SHORTER_WARNING      RES_MESSAGE +    162
#define RES_IMAGE_PERC_COMPLETE          RES_MESSAGE +    163
#define RES_IMAGE_BACKED_UP              RES_MESSAGE +    164
#define RES_IMAGE_RESTORED               RES_MESSAGE +    165
#define RES_IMAGE_VERIFIED               RES_MESSAGE +    166
#define RES_IMAGE_DIFFERENT              RES_MESSAGE +    167
#define RES_IMAGE_RESTORE_ERROR          RES_MESSAGE +    168
#define RES_ABORT_RETRY_FAIL             RES_MESSAGE +    169
#define RES_FAIL_KEY                     RES_MESSAGE +    170
#define RES_FAIL_STRING                  RES_MESSAGE +    171
#define RES_ABORT_KEY                    RES_MESSAGE +    172
#define RES_ABORT_STRING                 RES_MESSAGE +    173
#define RES_RETRY_KEY                    RES_MESSAGE +    174
#define RES_RETRY_STRING                 RES_MESSAGE +    175
#define RES_MISSING_NKS                  RES_MESSAGE +    176
#define RES_MISSING_RSS                  RES_MESSAGE +    177
#define RES_RESTORE_BINDERY              RES_MESSAGE +    178
#define RES_RESTORE_SECURITY             RES_MESSAGE +    179
#define RES_RESTORE_RECOVER              RES_MESSAGE +    180
#define RES_RECOVERED_FILE               RES_MESSAGE +    181
#define RES_RECOVERED_DIR                RES_MESSAGE +    182
#define RES_DATA_LOST                    RES_MESSAGE +    183
#define RES_TENSION_STATUS_LABEL         RES_MESSAGE +    184
#define RES_ERASE_STATUS_LABEL           RES_MESSAGE +    185
#define RES_ERASE_QUEST                  RES_MESSAGE +    186
#define RES_TENSION_STARTED              RES_MESSAGE +    187
#define RES_TENSION_COMPLETED            RES_MESSAGE +    188
#define RES_ERASE_STARTED                RES_MESSAGE +    189
#define RES_ERASE_COMPLETED              RES_MESSAGE +    190
#define RES_SEC_ERASE_STARTED            RES_MESSAGE +    191
#define RES_SEC_ERASE_COMPLETED          RES_MESSAGE +    192
#define RES_CATALOG_STARTED              RES_MESSAGE +    193
#define RES_CATALOG_COMPLETED            RES_MESSAGE +    194
#define RES_DISPLAY_TAPE                 RES_MESSAGE +    195
#define RES_UNCATALOGED_TAPE             RES_MESSAGE +    196
#define RES_CATALOGED_SET                RES_MESSAGE +    197
#define RES_CATALOGED_SETS               RES_MESSAGE +    198
#define RES_CATALOG_QUEST                RES_MESSAGE +    199
#define RES_ALREADY_CATALOGED_SET        RES_MESSAGE +    200
#define RES_CATALOGING_ITEMS             RES_MESSAGE +    201
#define RES_CATALOG_NO_ITEMS             RES_MESSAGE +    202
#define RES_CATALOG_COMPACT_LABEL        RES_MESSAGE +    203
#define RES_CATALOG_COMPACT_PROMPT       RES_MESSAGE +    204
#define RES_CATALOG_INFO                 RES_MESSAGE +    205
#define RES_CATALOG_SIZE                 RES_MESSAGE +    206
#define RES_CATALOG_COMPACT_STARTED      RES_MESSAGE +    207
#define RES_CATALOG_COMPACT_COMPLETED    RES_MESSAGE +    208
#define RES_AVAILABLE_DISK_SPACE         RES_MESSAGE +    209
#define RES_CATALOG_TAPE_FAMILY          RES_MESSAGE +    210
#define RES_CATALOG_BSET                 RES_MESSAGE +    211
#define RES_DELETE_QUEST                 RES_MESSAGE +    212
#define RES_PROMPT_VERIFY_BACKUP         RES_MESSAGE +    213
#define RES_PROMPT_VERIFY_RESTORE        RES_MESSAGE +    214
#define RES_MESSAGE_TITLE                RES_MESSAGE +    215
#define RES_NEWLY_CATALOGED_SET          RES_MESSAGE +    216
#define RES_CAT_NO_TAPES_MSG             RES_MESSAGE +    217
#define RES_IMAGE_FOUND                  RES_MESSAGE +    218
#define RES_NUM_IMAGE_FOUND              RES_MESSAGE +    219
#define RES_PAGE_UP_ONLY                 RES_MESSAGE +    220
#define RES_PAGE_DOWN_ONLY               RES_MESSAGE +    221
#define RES_PAGE_UP_AND_DOWN             RES_MESSAGE +    222
#define RES_NONE_SOME_ALL                RES_MESSAGE +    223
#define RES_DRIVE_TYPES                  RES_MESSAGE +    224
#define RES_COMPACTION_OUT_OF_SPACE      RES_MESSAGE +    225
#define RES_CATALOG_CLEANUP_STARTED      RES_MESSAGE +    226
#define RES_CATALOG_CLEANUP_COMPLETED    RES_MESSAGE +    227
#define RES_CATALOGING_IN_PROGRESS       RES_MESSAGE +    228
#define RES_CATALOGING_COMPLETE          RES_MESSAGE +    229
#define RES_TAPE_SELECTION_STATUS        RES_MESSAGE +    230
#define RES_TARGET_DRIVE_WINDOW          RES_MESSAGE +    231
#define RES_ALL_FILES_SELECTED           RES_MESSAGE +    232
#define RES_NO_FILES_SELECTED            RES_MESSAGE +    233
#define RES_SOME_FILES_SELECTED          RES_MESSAGE +    234
#define RES_SEARCHING_FILES              RES_MESSAGE +    235
#define RES_SEARCH_NO_ITEMS_FOUND        RES_MESSAGE +    236
#define RES_CHECKING_DRIVE               RES_MESSAGE +    237
#define RES_POSITION_TAPE                RES_MESSAGE +    238
#define RES_CATALOG_ERASED_TAPE          RES_MESSAGE +    239
#define RES_CATALOG_NO_INFORMATION       RES_MESSAGE +    240
#define RES_TAPE_FILE_SELECT_TAPE        RES_MESSAGE +    241
#define RES_TAPE_FILE_SELECT_LINE2       RES_MESSAGE +    242
#define RES_FILE_OPEN_ERROR              RES_MESSAGE +    243
#define RES_NOTICES_TITLE                RES_MESSAGE +    244
#define RES_BUILDING_DIRECTORY           RES_MESSAGE +    245
#define RES_VM_CRITICAL_ERROR            RES_MESSAGE +    246
#define RES_EMPTY_TREE_WARNING           RES_MESSAGE +    247
#define RES_ERASE_CAT_WARNING            RES_MESSAGE +    248
#define RES_CONTINU_FILE_WARNING         RES_MESSAGE +    249
#define RES_CONTINU_FILE_PROMPT          RES_MESSAGE +    250
#define RES_CONTINU_IMAGE_WARNING        RES_MESSAGE +    251
#define RES_CATALOG_REPAIR_LABEL         RES_MESSAGE +    252
#define RES_CATALOG_REPAIR_STARTED       RES_MESSAGE +    253
#define RES_CATALOG_REPAIR_COMPLETE      RES_MESSAGE +    254
#define RES_CATALOG_RECREATE_PROMPT      RES_MESSAGE +    255
#define RES_CATALOG_REPAIR_OUT_OF_SPACE  RES_MESSAGE +    256
#define RES_INSERT_MULTI_TAPES           RES_MESSAGE +    257
#define RES_TAPE_DRIVE_NAME              RES_MESSAGE +    258
#define RES_PRODUCT_DESCRIPT             RES_MESSAGE +    259
#define RES_WAIT_AND_REPLACE_TAPE        RES_MESSAGE +    260
#define RES_MAKE_ANOTHER_COPY            RES_MESSAGE +    261
#define RES_ANY_KEY_ESC                  RES_MESSAGE +    262
#define RES_DIRECTORY_DIFFERENT          RES_MESSAGE +    263
#define RES_IMAGE_LOGING_WARNING         RES_MESSAGE +    264
#define RES_CATALOG_LEVEL_ZERO           RES_MESSAGE +    265
#define RES_SECURITY_DIFFERENCE          RES_MESSAGE +    266
#define RES_SECURITY_DIFFERENCES         RES_MESSAGE +    267
#define RES_INSUFFICIENT_PRIVILEGE       RES_MESSAGE +    268
#define RES_INSUFFICIENT_DISK_SPACE      RES_MESSAGE +    269
#define RES_ERROR_CREATING_FILE          RES_MESSAGE +    270
#define RES_ERROR_CREATING_DIR           RES_MESSAGE +    271
#define RES_ERROR_RESTORING_AFP_FILE     RES_MESSAGE +    272
#define RES_ERROR_RESTORING_FILE         RES_MESSAGE +    273
#define RES_ERROR_RESTORING_DIR          RES_MESSAGE +    274
#define RES_ERROR_RESTORING_FILE_SEC     RES_MESSAGE +    275
#define RES_ERROR_RESTORING_DIR_SEC      RES_MESSAGE +    276
#define RES_ERROR_RESTORING_TRUSTEE_SEC  RES_MESSAGE +    277
#define RES_EMPTY_DIRECTORY_MESSAGE      RES_MESSAGE +    278
#define RES_CATALOGING_AT_LEVEL_ZERO     RES_MESSAGE +    279
#define RES_CATALOG_DELETE_COMPLETE      RES_MESSAGE +    280
#define RES_CATALOGING_TERMINATED        RES_MESSAGE +    281
#define RES_SEARCHING_NEXT_FILE          RES_MESSAGE +    282
#define RES_MENU_CONTINUE                RES_MESSAGE +    283
#define RES_TEMP_CAT_WARNING             RES_MESSAGE +    284
#define RES_SKIP_CONTINUE                RES_MESSAGE +    285
#define RES_OS_FILE_INFO_DIFFERENT       RES_MESSAGE +    286
#define RES_FULLY_CATALOGED_TAPE         RES_MESSAGE +    287
#define RES_TAPE_FILE_SELECT_BSET        RES_MESSAGE +    288
#define RES_BACKUP_SET_BYTES             RES_MESSAGE +    289
#define RES_NOT_CATALOGING_WARNING       RES_MESSAGE +    290
#define RES_MORE                         RES_MESSAGE +    291
#define RES_CONVERT_MAC_BSET             RES_MESSAGE +    292
#define RES_SCRIPT_MARKER                RES_MESSAGE +    293
#define RES_NO_DRIVER_LOADED             RES_MESSAGE +    294
#define RES_SERVER_LOGOUT_DENIED         RES_MESSAGE +    295
#define RES_BAD_ATTR_READ                RES_MESSAGE +    296
#define RES_DISPLAY_BSD_VCB              RES_MESSAGE +    297
#define RES_DETERMINING_LBAS             RES_MESSAGE +    298
#define RES_SKIPPED_DEVICE               RES_MESSAGE +    299
#define RES_ERROR_DURING_OPERATION       RES_MESSAGE +    300
#define RES_ERROR_FILE_TO_EXAMINE        RES_MESSAGE +    301
#define RES_FILE_SECURITY_DIFF           RES_MESSAGE +    302
#define RES_FILE_EA_DIFF                 RES_MESSAGE +    303
#define RES_FILE_RES_DIFF                RES_MESSAGE +    304
#define RES_SET_LAD_STARTED              RES_MESSAGE +    305
#define RES_SET_LAD_COMPLETED            RES_MESSAGE +    306
#define RES_ERROR_SETTING_LAD            RES_MESSAGE +    307
#define RES_NOERROR_DURING_OPERATION     RES_MESSAGE +    308
#define RES_NO_TRANSFER_APPEND           RES_MESSAGE +    309
#define RES_MKT_VER_MAJ                  RES_MESSAGE +    310
#define RES_MKT_VER_MIN                  RES_MESSAGE +    311
#define RES_ENG_VER_MAJ                  RES_MESSAGE +    312
#define RES_ENG_VER_MIN                  RES_MESSAGE +    313
#define RES_EMPTY_DIR_WARNING            RES_MESSAGE +    314
#define RES_BACKUP_BINDERY               RES_MESSAGE +    315
#define RES_RECOVERY_PROMPT              RES_MESSAGE +    316
#define RES_USER_INPUT                   RES_MESSAGE +    317
#define RES_CONNECTION_PASSWORD          RES_MESSAGE +    318
#define RES_REPLACE_SCRIPT               RES_MESSAGE +    319
#define RES_TAPE_NAME                    RES_MESSAGE +    320
#define RES_BACKUP_SET_NAME              RES_MESSAGE +    321
#define RES_BACKUP_SET_DESCR             RES_MESSAGE +    322
#define RES_DISPLAY_VERIFY_INFO          RES_MESSAGE +    323
#define RES_CATALOG_TITLE                RES_MESSAGE +    324        /* open for use */
#define RES_DELETE_TITLE                 RES_MESSAGE +    325        /* open for use */
#define RES_ERASE_BAD_TAPE               RES_MESSAGE +    326
#define RES_TARGET_TRANSFER_TITLE        RES_MESSAGE +    327
#define RES_ERASE_FOREIGN_TAPE           RES_MESSAGE +    328
#define RES_ERASE_BLANK_TAPE             RES_MESSAGE +    329
#define RES_ERASE_NO_TAPE                RES_MESSAGE +    330
#define RES_ERASE_TAPE_INFO1             RES_MESSAGE +    331
#define RES_ERASE_TAPE_INFO2             RES_MESSAGE +    332
#define RES_ABORT_QUESTION               RES_MESSAGE +    333
#define RES_PROCESS_ABORTED              RES_MESSAGE +    334
#define RES_ERASE_DRIVE_BUSY             RES_MESSAGE +    335
#define RES_NEED_NEXT_TAPE               RES_MESSAGE +    336
#define RES_TAPE_FULL                    RES_MESSAGE +    337
#define RES_RESTORE_DESC_1               RES_MESSAGE +    338
#define RES_RETENSION_MESSAGE            RES_MESSAGE +    339
#define RES_TITLE_NEW_LINE               RES_MESSAGE +    340
#define RES_NEXT_SET                     RES_MESSAGE +    341
#define RES_NO_NEXT_SET                  RES_MESSAGE +    342
#define RES_FOUND_BSET                   RES_MESSAGE +    343
#define RES_ERASE_POLL_DRIVE_DISABLED    RES_MESSAGE +    344
#define RES_POLL_DRIVE_BAD_TAPE          RES_MESSAGE +    345
#define RES_POLL_DRIVE_GOOFY_TAPE        RES_MESSAGE +    346
#define RES_KEEP_CURRENT_SETTINGS        RES_MESSAGE +    347
#define RES_ERASE_PWDB                   RES_MESSAGE +    348
#define RES_PWDB_DISABLED                RES_MESSAGE +    349
#define RES_PWDB_BAD_CONFIRM             RES_MESSAGE +    350
#define RES_INIT_FILE_SYSTEM             RES_MESSAGE +    351
#define RES_INIT_HARDWARE                RES_MESSAGE +    352
#define RES_INIT_APPLICATION             RES_MESSAGE +    353
#define RES_APPLICATION_INIT             RES_MESSAGE +    354
#define RES_INIT_VLM                     RES_MESSAGE +    355
#define RES_OPENING_LOG_NAME             RES_MESSAGE +    356
#define RES_ERROR_ATTACHING              RES_MESSAGE +    357
#define RES_CLOSING_LOG_NAME             RES_MESSAGE +    358
#define RES_ALREADY_FULLY_CATALOGED      RES_MESSAGE +    359
#define RES_VLM_BLANK_TAPE               RES_MESSAGE +    360
#define RES_VLM_FOREIGN_TAPE             RES_MESSAGE +    361
#define RES_VLM_BAD_TAPE                 RES_MESSAGE +    362
#define RES_FILE_WAS_SKIPPED             RES_MESSAGE +    363
#define RES_FILE_WAS_SKIPPED_USER        RES_MESSAGE +    364
#define RES_IMAGE_BACKUP                 RES_MESSAGE +    365
#define RES_VLM_NO_TAPE                  RES_MESSAGE +    366
#define RES_VLM_BUSY_DRIVE               RES_MESSAGE +    367
#define RES_VLM_UNFORMATED_TAPE          RES_MESSAGE +    368
#define RES_FILE_DETAIL                  RES_MESSAGE +    369
#define RES_BACKED_UP_DIRS_FILE          RES_MESSAGE +    370
#define RES_CONTINUE_BACKUP_ABORT        RES_MESSAGE +    371     // chs:02-08-93
#define RES_NEW_PROCESSED_BYTES          RES_MESSAGE +    372
#define RES_BYTES_PROCESSED              RES_MESSAGE +    373
#define RES_BYTES_PROCESSED_HOUR         RES_MESSAGE +    374
#define RES_BYTES_PROCESSED_HOURS        RES_MESSAGE +    375
#define RES_BYTES_PROCESSED_MINUTE1      RES_MESSAGE +    376
#define RES_BYTES_PROCESSED_MINUTES1     RES_MESSAGE +    377
#define RES_BYTES_PROCESSED_MINUTE2      RES_MESSAGE +    378
#define RES_BYTES_PROCESSED_MINUTES2     RES_MESSAGE +    379
#define RES_BYTES_PROCESSED_SECOND       RES_MESSAGE +    380
#define RES_BYTES_PROCESSED_SECONDS      RES_MESSAGE +    381
#define RES_DISPLAY_VOLUME_1             RES_MESSAGE +    382
#define RES_CONTINUE_RESTORE_ABORT       RES_MESSAGE +    383     // chs:02-08-93

//
// ABORT ERROR STRING
//

#define RES_CURRENT_FILE                 RES_MESSAGE +    384
#define RES_UNUSED                       RES_MESSAGE +    385
#define RES_BACKUP_ABORT_EOF             RES_MESSAGE +    386
#define RES_RESTORE_ABORT_EOF            RES_MESSAGE +    387

#define RES_RESTORED_DIRS_FILE           RES_MESSAGE +    388     // chs:02-11-93

#define RES_FORMAT_TAPE_WARNING          RES_MESSAGE +    389
#define RES_FORMAT_DIALOG_TITLE          RES_MESSAGE +    390

#define RES_FORMAT_STARTED               RES_MESSAGE +    391
#define RES_FORMAT_COMPLETED             RES_MESSAGE +    392
#define RES_ABORT_OPERATION              RES_MESSAGE +    393

#define RES_SEARCHING_FOR_EOD            RES_MESSAGE +    394
#define RES_DRIVE_ERROR_DETECTED         RES_MESSAGE +    395
#define RES_OPERATION_COMPLETED          RES_MESSAGE +    396      // chs:04-30-93
#define RES_TAPE_FULL_REWOUND            RES_MESSAGE +    397      // chs:05-03-93
#define RES_NEED_NEXT_TAPE_REWOUND       RES_MESSAGE +    398      // chs:05-10-93
#define RES_INSERT_NEXT_TAPE_REWOUND     RES_MESSAGE +    399      // chs:05-10-93
#define RES_ACTIVE_FILES_RESTORED        RES_MESSAGE +    400
#define RES_SAME_TAPE_FAMILY             RES_MESSAGE +    401      // chs:05-20-93
#define RES_FOREIGN_TAPE_MSG2            RES_MESSAGE +    402

#define RES_BACKUP_ABORT_PART2           RES_MESSAGE +    403      // chs:05-25-93
#define RES_RESTORE_ABORT_PART2          RES_MESSAGE +    404      // chs:05-25-93
#define RES_DELETE_DIRS_FILE             RES_MESSAGE +    405      // chs:05-26-93

#define RES_HW_COMP_FAILURE              RES_MESSAGE +    406      // chs:06-01-93
#define RES_HW_UNCOMP_FAILURE            RES_MESSAGE +    407      // chs:06-01-93
#define RES_RESUME_PROCESS               RES_MESSAGE +    408      // chs:07-13-93

#define RES_USE_TAPE_CATALOGS            RES_MESSAGE +    409
#define RES_VLM_ECC_TAPE                 RES_MESSAGE +    410
#define RES_VLM_FUTURE_TAPE              RES_MESSAGE +    411

#define RES_NOAPPEND                     RES_MESSAGE +    412
#define RES_NODISKSPACE                  RES_MESSAGE +    413

#define RES_COMM_FAILURE                 RES_MESSAGE +    414
#define RES_FILE_NOT_DELETED             RES_MESSAGE +    415
#define RES_DIRECTORY_NOT_DELETED        RES_MESSAGE +    416
#define RES_VLM_GOOFY_TAPE               RES_MESSAGE +    417
#define RES_USESYPLFLAG                  RES_MESSAGE +    418
#define RES_VLM_SQL_TAPE                 RES_MESSAGE +    419

// LOADER
#define RES_LDR_DUPLICATE_MAG_NAME       RES_MESSAGE +    420
#define RES_LDR_INVALID_MAG_SIZE         RES_MESSAGE +    421
#define RES_LDR_GROUP_NAME_NOT_FOUND     RES_MESSAGE +    422
#define RES_LDR_DUPLICATE_GROUP_NAME     RES_MESSAGE +    423
#define RES_LDR_INVALID_STATING_SLOT     RES_MESSAGE +    424
#define RES_LDR_INVALID_NUMBER_SLOTS     RES_MESSAGE +    425
#define RES_LDR_SLOT_IN_USE              RES_MESSAGE +    426
#define RES_LDR_BAD_MAG_NAME             RES_MESSAGE +    427
#define RES_LDR_WRONG_CONF               RES_MESSAGE +    428
#define RES_LDR_ERASING_GROUP            RES_MESSAGE +    429
#define RES_LDR_OPERATION_ABORTED        RES_MESSAGE +    430
#define RES_LDR_STARTING_TAPE_GROUP      RES_MESSAGE +    431
#define RES_LDR_DEFAULT_SLOTS            RES_MESSAGE +    432
#define RES_LDR_DEFAULT_SLOT             RES_MESSAGE +    433
#define RES_LDR_NO_SELECTION             RES_MESSAGE +    434
// LOADER

#endif
