#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

// 下面是一些功能的控制，如果屏蔽该控制，则表示不使用这个功能
// 如果该行前面有 "//" 则表示被屏蔽，要屏蔽该功能，则加入 "//"

//#define SHOW_THANKYOU         /* 显示源代码提供者信息 */
// 水源添加的功能
#define MAIL_COMBINE            /* mail 做合集 */
//#define SPECIALBOARD "WelcomeNewer" /*特殊版 */
//#define SPECIALID                             /*特殊ID */
//#define SPECIALID1 "freshboy"
//#define SPECIALID2 "freshgirl"

/*  注册相关部分代码的 define */
/*  一鼓作气把注册这部分做了一些变更， 这样可能可以满足比较多朋友的需要 */
#define NEWCOMERREPORT          /* 新手上路在 newcomers 板自动发文 */
//#define MAILCHECK             /* 提供邮件注册功能 */
//#define CODE_VALID            /* 暗码认证 */
//#define SAVELIVE              /* 锁定帐号 防止用户长时间不上线而死亡*/
//#define AUTOGETPERM           /* 无需注册即获取基本权限 */
//#define PASSAFTERTHREEDAYS    /* 新手上路三天限制 */
//#define MAILCHANGED           /* 修改 e-mail 后要求重新注册确认 */

/* 系统安全相关代码的 define */
//#define MUDCHECK_BEFORELOGIN  /* 登陆前按键确认 */
//#define CHECK_SYSTEM_PASS     /* 站长级 ID 上站询问系统密码 */
//#define SYSOPLOGINPROTECT     /* SYSOP 帐号登陆安全 */

/* 某些限制性代码的相关 define */
#define BOARD_CONTROL           /* 提供成员版面设置功能 Ctrl+K */
//#define KEEP_DELETED_HEADER   /* 保留删除文章记录 */
//#define BELL_DELAY_FILTER     /* 过滤文章中的响铃和延时控制 */
#define CHECK_LESS60SEC         /* 60 秒内重复 login 时要求按键确认身份 */
//#define MARK_X_FLAG           /* 将灌水文章加上 'X' 标记 */

/* 一般不需要变更的 define */
//#define SHOWMETOFRIEND        /* 环顾四方的是否为对方好友的显示 */
#define BBSD                    /* 使用 BBS daemon, 不使用 bbsrf */
#define ALLOWAUTOWRAP           /* 启用自动排版功能 */
#define ALLOWSWITCHCODE         /* 启用 GB 码 <==> Big5 码 切换 */
#define USE_NOTEPAD             /* 使用留言版 */
#define INTERNET_EMAIL          /* 发送 InterNet Mail */
#define CHKPASSWDFORK           /* 系统检查密码时采用 fork 进行 */
#define COLOR_POST_DATE         /* 文章日期颜色 */
#define TALK_LOG                /* 聊天纪录功能 */
#define RNDSIGN                 /* 乱数签名档 */
//#define FOR_DOMAIN_NAME         /* 登陆时进行域名反查 */ /*代码设计不良,使用过多内存,暂停使用 by jawahh*/
#define NEW_CREATE_BRD          // 新的创建/修改讨论区的过程
// 适合于对系统不熟的用户
/* 看站长的喜好啦， 想如何就如何吧， 当然你要懂这个啦，呵呵 */
//#define MSG_CANCLE_BY_CTRL_C  /* 用 ctrl-c 来否略讯息 */
#define LOG_MY_MESG             /* 讯息纪录中纪录自己所发出的讯息 */
#define BIGGER_MOVIE            /* 加大活动看板空间 (七行) */
//#define ALWAYS_SHOW_BRDNOTE   /* 每次进板都会 show 出进板画面 */
#endif
/* _FUNCTIONS_H_ */
