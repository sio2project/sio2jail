#pragma once

/**
 * Simple preprocessor helper library based on various
 * tutorials and ansers from StackOverflow.
 */

#define EVAL(...) _EVAL_5(_EVAL_5(_EVAL_5(_EVAL_5(__VA_ARGS__))))
#define _EVAL_5(...) _EVAL_4(_EVAL_4(_EVAL_4(_EVAL_4(__VA_ARGS__))))
#define _EVAL_4(...) _EVAL_3(_EVAL_3(_EVAL_3(_EVAL_3(__VA_ARGS__))))
#define _EVAL_3(...) _EVAL_2(_EVAL_2(_EVAL_2(_EVAL_2(__VA_ARGS__))))
#define _EVAL_2(...) _EVAL_1(_EVAL_1(_EVAL_1(_EVAL_1(__VA_ARGS__))))
#define _EVAL_1(...) _EVAL_0(_EVAL_0(_EVAL_0(_EVAL_0(__VA_ARGS__))))
#define _EVAL_0(...) __VA_ARGS__

#define CAT(a, ...) _CAT(a, __VA_ARGS__)
#define _CAT(a, ...) a ## __VA_ARGS__

#define EMPTY()
#define DEFER(...) __VA_ARGS__ EMPTY()
#define DEFER2(...) __VA_ARGS__ DEFER(EMPTY) ()

/**
 * Evaluates 1 when argument list is empty, to 0 otherwise
 */
#define VA_EMPTY(...) _VA_EMPTY(_, ## __VA_ARGS__, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1)
#define _VA_EMPTY(_, _1, _2, _3, _4, _5, _6, _7, _8, _9, _a, _b, _c, _d, _e, X, ...) X

/**
 * Evaluates to list size in hex, from 0 to e.
 */
#define VA_SIZE(...) _VA_SIZE(_, ## __VA_ARGS__, e, d, c, b, a, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define _VA_SIZE(_, _1, _2, _3, _4, _5, _6, _7, _8, _9, _a, _b, _c, _d, _e, X, ...) X

/**
 * Simple map function.
 */
#define MAP(...) \
    EVAL(_MAP(__VA_ARGS__))
#define _MAP(ACTION, ...) \
    DEFER2(CAT(_MAP_, VA_EMPTY(__VA_ARGS__)))(ACTION, __VA_ARGS__)
#define _MAP_1(...)
#define _MAP_0(ACTION, X, ...) \
    ACTION(X) DEFER2(_MAP)(ACTION, __VA_ARGS__)
