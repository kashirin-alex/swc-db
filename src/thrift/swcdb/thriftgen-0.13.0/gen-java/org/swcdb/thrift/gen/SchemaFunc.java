/**
 * Autogenerated by Thrift Compiler (0.13.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
package org.swcdb.thrift.gen;


public enum SchemaFunc implements org.apache.thrift.TEnum {
  CREATE(3),
  DELETE(5),
  MODIFY(7);

  private final int value;

  private SchemaFunc(int value) {
    this.value = value;
  }

  /**
   * Get the integer value of this enum value, as defined in the Thrift IDL.
   */
  public int getValue() {
    return value;
  }

  /**
   * Find a the enum type by its integer value, as defined in the Thrift IDL.
   * @return null if the value is not found.
   */
  @org.apache.thrift.annotation.Nullable
  public static SchemaFunc findByValue(int value) { 
    switch (value) {
      case 3:
        return CREATE;
      case 5:
        return DELETE;
      case 7:
        return MODIFY;
      default:
        return null;
    }
  }
}
