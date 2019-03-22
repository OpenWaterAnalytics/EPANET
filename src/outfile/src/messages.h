/*
 * messages.h - EPANET
 *
 *  Created on: June 1, 2017
 *
 *  Author:     Michael E. Tryby
 *              US EPA - ORD/NRMRL
 */

#ifndef MESSAGES_H_
#define MESSAGES_H_
/*------------------- Error Messages --------------------*/
#define MSG_MAXLEN 53

#define WARN10 "Warning: model run issued warnings"

#define ERR411 "Input Error 411: no memory allocated for results"
#define ERR412 "Input Error 412: binary file hasn't been opened"
#define ERR421 "Input Error 421: invalid parameter code"
#define ERR422 "Input Error 422: reporting period index out of range"
#define ERR423 "Input Error 423: element index out of range"

#define ERR434 "File Error 434: unable to open binary file"
#define ERR435 "File Error 435: invalid binary file type"
#define ERR436 "File Error 436: no results in binary file"

#define ERRERR "Error: An unknown error has occurred"

#endif /* MESSAGES_H_ */
