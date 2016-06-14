/*-----------------------------------------------------------------------------
This references from Coroutines in C by Simon Tatham

http://www.chiark.greenend.org.uk/~sgtatham/coroutines.html
-----------------------------------------------------------------------------*/

#ifndef __COROUTINE_H__
#define __COROUTINE_H__

#define crBegin() static int _cr_state=0; switch(_cr_state) { case 0:
#define crReturn(x) do {_cr_state=__LINE__; return x; case __LINE__:;} while(0)
#define crFinish() }

#endif
