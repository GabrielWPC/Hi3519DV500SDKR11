/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef SAMPLE_LOG_H
#define SAMPLE_LOG_H

#define sample_log(format, arg...)  printf(format, ##arg)
#define sample_err(format, arg...) printf("Error: [%s-%d]" format, __FUNCTION__, __LINE__, ##arg)

#ifndef sample_chk_return
#define sample_chk_return(cond, err_ret) do {   \
    if (cond) { \
        sample_err("%s\n", ##cond);   \
        return err_ret; \
    }   \
} while (0)
#endif

#ifndef sample_chk_expr_return
#define sample_chk_expr_return(expr, expected_ret) do { \
    int _ret = expr;  \
    if (_ret != (expected_ret)) { \
        sample_err("%s return 0x%x\n", #expr, _ret);    \
        return _ret;    \
    }   \
} while (0)
#endif

#ifndef sample_chk_expr_with_return
#define sample_chk_expr_with_return(expr, expected_ret, err_ret) do {   \
    int _ret = expr;    \
    if (_ret != (expected_ret)) { \
        sample_err("%s return 0x%x\n", #expr, _ret);    \
        return (err_ret);    \
    }   \
} while (0)
#endif

#ifndef sample_chk_expr_goto
#define sample_chk_expr_goto(expr, expected_ret, label) do { \
    int _ret = expr;  \
    if (_ret != (expected_ret)) { \
        sample_err("%s return 0x%x\n", #expr, _ret);    \
        ret = _ret;  \
        goto label;    \
    }   \
} while (0)
#endif

#ifndef sample_chk_expr_goto_with_ret
#define sample_chk_expr_goto_with_ret(expr, expected_ret, ret, err_ret, label) do { \
    int _ret = expr;  \
    if (_ret != (expected_ret)) { \
        sample_err("%s return 0x%x\n", #expr, _ret);    \
        ret = (err_ret);  \
        goto label;    \
    }   \
} while (0)
#endif

#endif