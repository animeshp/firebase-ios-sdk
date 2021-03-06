/*
 * Copyright 2017 Google
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#import <Foundation/Foundation.h>

#import "Firestore/Source/Model/FSTDocumentKey.h"
#import "Firestore/third_party/Immutable/FSTImmutableSortedDictionary.h"

#include "Firestore/core/src/firebase/firestore/model/database_id.h"
#include "Firestore/core/src/firebase/firestore/model/field_mask.h"
#include "Firestore/core/src/firebase/firestore/model/field_path.h"
#include "Firestore/core/src/firebase/firestore/model/field_value.h"
#include "Firestore/core/src/firebase/firestore/model/field_value_options.h"

@class FIRTimestamp;

namespace model = firebase::firestore::model;

NS_ASSUME_NONNULL_BEGIN

/** The order of types in Firestore; this order is defined by the backend. */
typedef NS_ENUM(NSInteger, FSTTypeOrder) {
  FSTTypeOrderNull,
  FSTTypeOrderBoolean,
  FSTTypeOrderNumber,
  FSTTypeOrderTimestamp,
  FSTTypeOrderString,
  FSTTypeOrderBlob,
  FSTTypeOrderReference,
  FSTTypeOrderGeoPoint,
  FSTTypeOrderArray,
  FSTTypeOrderObject,
};

/**
 * Abstract base class representing an immutable data value as stored in Firestore. FSTFieldValue
 * represents all the different kinds of values that can be stored in fields in a document.
 *
 * Supported types are:
 *  - Null
 *  - Boolean
 *  - Long
 *  - Double
 *  - Timestamp
 *  - ServerTimestamp (a sentinel used in uncommitted writes)
 *  - String
 *  - Binary
 *  - (Document) References
 *  - GeoPoint
 *  - Array
 *  - Object
 */
@interface FSTFieldValue<__covariant T> : NSObject

/**
 * Returns the 'type' of this FSTFieldValue. Used for RTTI (rather than isKindOfClass)
 * to ease migration to C++.
 */
@property(nonatomic, assign, readonly) model::FieldValue::Type type;

/** Returns the FSTTypeOrder for this value. */
@property(nonatomic, assign, readonly) FSTTypeOrder typeOrder;

/**
 * Converts an FSTFieldValue into the value that users will see in document snapshots.
 *
 * TODO(mikelehen): This conversion should probably happen at the API level and right now `value` is
 * used inappropriately in the serializer implementation, etc.  We need to do some reworking.
 */
- (T)value;

/**
 * Converts an FSTFieldValue into the value that users will see in document snapshots.
 *
 * Options can be provided to configure the deserialization of some field values (such as server
 * timestamps).
 */
- (T)valueWithOptions:(const model::FieldValueOptions &)options;

/** Compares against another FSTFieldValue. */
- (NSComparisonResult)compare:(FSTFieldValue *)other;

/** Accesses this FSTFieldValue as an integer. */
- (int64_t)integerValue;

- (bool)isNAN;

/** Accesses this FSTFieldValue as an double. */
- (double)doubleValue;

@end

/**
 * A structured object value stored in Firestore.
 */
// clang-format off
@interface FSTObjectValue : FSTFieldValue < NSDictionary<NSString *, id> * >

- (instancetype)init NS_UNAVAILABLE;
// clang-format on

/** Returns an empty FSTObjectValue. */
+ (instancetype)objectValue;

/**
 * Initializes this FSTObjectValue with the given dictionary.
 */
- (instancetype)initWithDictionary:(NSDictionary<NSString *, FSTFieldValue *> *)value;

/**
 * Initializes this FSTObjectValue with the given immutable dictionary.
 */
- (instancetype)initWithImmutableDictionary:
    (FSTImmutableSortedDictionary<NSString *, FSTFieldValue *> *)value NS_DESIGNATED_INITIALIZER;

- (FSTImmutableSortedDictionary<NSString *, FSTFieldValue *> *)internalValue;

/** Returns the value at the given path if it exists. Returns nil otherwise. */
- (nullable FSTFieldValue *)valueForPath:(const model::FieldPath &)fieldPath;

/**
 * Returns a new object where the field at the named path has its value set to the given value.
 * This object remains unmodified.
 */
- (FSTObjectValue *)objectBySettingValue:(FSTFieldValue *)value
                                 forPath:(const model::FieldPath &)fieldPath;

/**
 * Returns a new object where the field at the named path has been removed. If any segment of the
 * path does not exist within this object's structure, no change is performed.
 */
- (FSTObjectValue *)objectByDeletingPath:(const model::FieldPath &)fieldPath;

/**
 * Applies this field mask to the provided object value and returns an object that only contains
 * fields that are specified in both the input object and this field mask.
 */
// TODO(mrschmidt): Once FieldValues are C++, move this to FieldMask to match other platforms.
- (FSTObjectValue *)objectByApplyingFieldMask:(const model::FieldMask &)fieldMask;
@end

/**
 * An array value stored in Firestore.
 */
// clang-format off
@interface FSTArrayValue : FSTFieldValue < NSArray <id> * >

- (instancetype)init NS_UNAVAILABLE;
// clang-format on

/**
 * Initializes this instance with the given array of wrapped values.
 *
 * @param value An immutable array of FSTFieldValue objects. Caller is responsible for copying the
 *      value or releasing all references.
 */
- (instancetype)initWithValueNoCopy:(NSArray<FSTFieldValue *> *)value NS_DESIGNATED_INITIALIZER;

- (NSArray<FSTFieldValue *> *)internalValue;

@end

/**
 * A value that delegates to the c++ model::FieldValue.
 */
@interface FSTDelegateValue : FSTFieldValue <id>
+ (instancetype)delegateWithValue:(model::FieldValue &&)value;
- (const model::FieldValue &)internalValue;

- (const model::FieldValue::Reference &)referenceValue;
@end

NS_ASSUME_NONNULL_END
