//
//  hash.h
//  HealthApp
//
//  Created by Christopher Ray on 7/25/21.
//

#ifndef hash_h
#define hash_h

#include <stdint.h>

size_t murmurhash(const void *key, int len, uint32_t seed);

#endif /* hash_h */
