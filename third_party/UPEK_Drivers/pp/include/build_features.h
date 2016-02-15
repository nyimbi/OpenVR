/**
 * @file build_features.h
 * @brief Define the features to be active for this project
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 * Copyright AUTHENTEC 2010
 *
 */
#ifndef BUILD_FEATURES_H_
#define BUILD_FEATURES_H_

// (comment out features that are not to be defined, so they still show.)
#ifndef VERSION_LIB
#define DLL_EXPORT_FEATURE          TRUE
#define APP_DLL_IMPORT_FEATURE      FALSE
#endif

#define DLL_EXPORT_KEEP_FEATURE     TRUE
//#define DLL_EXPORT_DROP_FEATURE
//#define DLL_IMPORT_FEATURE

#endif  // #ifndef BUILD_FEATURES_H_
