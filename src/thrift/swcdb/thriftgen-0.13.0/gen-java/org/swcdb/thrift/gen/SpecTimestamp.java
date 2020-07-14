/**
 * Autogenerated by Thrift Compiler (0.13.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
package org.swcdb.thrift.gen;

@SuppressWarnings({"cast", "rawtypes", "serial", "unchecked", "unused"})
@javax.annotation.Generated(value = "Autogenerated by Thrift Compiler (0.13.0)", date = "2020-07-14")
public class SpecTimestamp implements org.apache.thrift.TBase<SpecTimestamp, SpecTimestamp._Fields>, java.io.Serializable, Cloneable, Comparable<SpecTimestamp> {
  private static final org.apache.thrift.protocol.TStruct STRUCT_DESC = new org.apache.thrift.protocol.TStruct("SpecTimestamp");

  private static final org.apache.thrift.protocol.TField COMP_FIELD_DESC = new org.apache.thrift.protocol.TField("comp", org.apache.thrift.protocol.TType.I32, (short)1);
  private static final org.apache.thrift.protocol.TField TS_FIELD_DESC = new org.apache.thrift.protocol.TField("ts", org.apache.thrift.protocol.TType.I64, (short)2);

  private static final org.apache.thrift.scheme.SchemeFactory STANDARD_SCHEME_FACTORY = new SpecTimestampStandardSchemeFactory();
  private static final org.apache.thrift.scheme.SchemeFactory TUPLE_SCHEME_FACTORY = new SpecTimestampTupleSchemeFactory();

  /**
   * 
   * @see Comp
   */
  public @org.apache.thrift.annotation.Nullable Comp comp; // required
  public long ts; // required

  /** The set of fields this struct contains, along with convenience methods for finding and manipulating them. */
  public enum _Fields implements org.apache.thrift.TFieldIdEnum {
    /**
     * 
     * @see Comp
     */
    COMP((short)1, "comp"),
    TS((short)2, "ts");

    private static final java.util.Map<java.lang.String, _Fields> byName = new java.util.HashMap<java.lang.String, _Fields>();

    static {
      for (_Fields field : java.util.EnumSet.allOf(_Fields.class)) {
        byName.put(field.getFieldName(), field);
      }
    }

    /**
     * Find the _Fields constant that matches fieldId, or null if its not found.
     */
    @org.apache.thrift.annotation.Nullable
    public static _Fields findByThriftId(int fieldId) {
      switch(fieldId) {
        case 1: // COMP
          return COMP;
        case 2: // TS
          return TS;
        default:
          return null;
      }
    }

    /**
     * Find the _Fields constant that matches fieldId, throwing an exception
     * if it is not found.
     */
    public static _Fields findByThriftIdOrThrow(int fieldId) {
      _Fields fields = findByThriftId(fieldId);
      if (fields == null) throw new java.lang.IllegalArgumentException("Field " + fieldId + " doesn't exist!");
      return fields;
    }

    /**
     * Find the _Fields constant that matches name, or null if its not found.
     */
    @org.apache.thrift.annotation.Nullable
    public static _Fields findByName(java.lang.String name) {
      return byName.get(name);
    }

    private final short _thriftId;
    private final java.lang.String _fieldName;

    _Fields(short thriftId, java.lang.String fieldName) {
      _thriftId = thriftId;
      _fieldName = fieldName;
    }

    public short getThriftFieldId() {
      return _thriftId;
    }

    public java.lang.String getFieldName() {
      return _fieldName;
    }
  }

  // isset id assignments
  private static final int __TS_ISSET_ID = 0;
  private byte __isset_bitfield = 0;
  public static final java.util.Map<_Fields, org.apache.thrift.meta_data.FieldMetaData> metaDataMap;
  static {
    java.util.Map<_Fields, org.apache.thrift.meta_data.FieldMetaData> tmpMap = new java.util.EnumMap<_Fields, org.apache.thrift.meta_data.FieldMetaData>(_Fields.class);
    tmpMap.put(_Fields.COMP, new org.apache.thrift.meta_data.FieldMetaData("comp", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.EnumMetaData(org.apache.thrift.protocol.TType.ENUM, Comp.class)));
    tmpMap.put(_Fields.TS, new org.apache.thrift.meta_data.FieldMetaData("ts", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.I64)));
    metaDataMap = java.util.Collections.unmodifiableMap(tmpMap);
    org.apache.thrift.meta_data.FieldMetaData.addStructMetaDataMap(SpecTimestamp.class, metaDataMap);
  }

  public SpecTimestamp() {
  }

  public SpecTimestamp(
    Comp comp,
    long ts)
  {
    this();
    this.comp = comp;
    this.ts = ts;
    setTsIsSet(true);
  }

  /**
   * Performs a deep copy on <i>other</i>.
   */
  public SpecTimestamp(SpecTimestamp other) {
    __isset_bitfield = other.__isset_bitfield;
    if (other.isSetComp()) {
      this.comp = other.comp;
    }
    this.ts = other.ts;
  }

  public SpecTimestamp deepCopy() {
    return new SpecTimestamp(this);
  }

  @Override
  public void clear() {
    this.comp = null;
    setTsIsSet(false);
    this.ts = 0;
  }

  /**
   * 
   * @see Comp
   */
  @org.apache.thrift.annotation.Nullable
  public Comp getComp() {
    return this.comp;
  }

  /**
   * 
   * @see Comp
   */
  public SpecTimestamp setComp(@org.apache.thrift.annotation.Nullable Comp comp) {
    this.comp = comp;
    return this;
  }

  public void unsetComp() {
    this.comp = null;
  }

  /** Returns true if field comp is set (has been assigned a value) and false otherwise */
  public boolean isSetComp() {
    return this.comp != null;
  }

  public void setCompIsSet(boolean value) {
    if (!value) {
      this.comp = null;
    }
  }

  public long getTs() {
    return this.ts;
  }

  public SpecTimestamp setTs(long ts) {
    this.ts = ts;
    setTsIsSet(true);
    return this;
  }

  public void unsetTs() {
    __isset_bitfield = org.apache.thrift.EncodingUtils.clearBit(__isset_bitfield, __TS_ISSET_ID);
  }

  /** Returns true if field ts is set (has been assigned a value) and false otherwise */
  public boolean isSetTs() {
    return org.apache.thrift.EncodingUtils.testBit(__isset_bitfield, __TS_ISSET_ID);
  }

  public void setTsIsSet(boolean value) {
    __isset_bitfield = org.apache.thrift.EncodingUtils.setBit(__isset_bitfield, __TS_ISSET_ID, value);
  }

  public void setFieldValue(_Fields field, @org.apache.thrift.annotation.Nullable java.lang.Object value) {
    switch (field) {
    case COMP:
      if (value == null) {
        unsetComp();
      } else {
        setComp((Comp)value);
      }
      break;

    case TS:
      if (value == null) {
        unsetTs();
      } else {
        setTs((java.lang.Long)value);
      }
      break;

    }
  }

  @org.apache.thrift.annotation.Nullable
  public java.lang.Object getFieldValue(_Fields field) {
    switch (field) {
    case COMP:
      return getComp();

    case TS:
      return getTs();

    }
    throw new java.lang.IllegalStateException();
  }

  /** Returns true if field corresponding to fieldID is set (has been assigned a value) and false otherwise */
  public boolean isSet(_Fields field) {
    if (field == null) {
      throw new java.lang.IllegalArgumentException();
    }

    switch (field) {
    case COMP:
      return isSetComp();
    case TS:
      return isSetTs();
    }
    throw new java.lang.IllegalStateException();
  }

  @Override
  public boolean equals(java.lang.Object that) {
    if (that == null)
      return false;
    if (that instanceof SpecTimestamp)
      return this.equals((SpecTimestamp)that);
    return false;
  }

  public boolean equals(SpecTimestamp that) {
    if (that == null)
      return false;
    if (this == that)
      return true;

    boolean this_present_comp = true && this.isSetComp();
    boolean that_present_comp = true && that.isSetComp();
    if (this_present_comp || that_present_comp) {
      if (!(this_present_comp && that_present_comp))
        return false;
      if (!this.comp.equals(that.comp))
        return false;
    }

    boolean this_present_ts = true;
    boolean that_present_ts = true;
    if (this_present_ts || that_present_ts) {
      if (!(this_present_ts && that_present_ts))
        return false;
      if (this.ts != that.ts)
        return false;
    }

    return true;
  }

  @Override
  public int hashCode() {
    int hashCode = 1;

    hashCode = hashCode * 8191 + ((isSetComp()) ? 131071 : 524287);
    if (isSetComp())
      hashCode = hashCode * 8191 + comp.getValue();

    hashCode = hashCode * 8191 + org.apache.thrift.TBaseHelper.hashCode(ts);

    return hashCode;
  }

  @Override
  public int compareTo(SpecTimestamp other) {
    if (!getClass().equals(other.getClass())) {
      return getClass().getName().compareTo(other.getClass().getName());
    }

    int lastComparison = 0;

    lastComparison = java.lang.Boolean.valueOf(isSetComp()).compareTo(other.isSetComp());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetComp()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.comp, other.comp);
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    lastComparison = java.lang.Boolean.valueOf(isSetTs()).compareTo(other.isSetTs());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetTs()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.ts, other.ts);
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    return 0;
  }

  @org.apache.thrift.annotation.Nullable
  public _Fields fieldForId(int fieldId) {
    return _Fields.findByThriftId(fieldId);
  }

  public void read(org.apache.thrift.protocol.TProtocol iprot) throws org.apache.thrift.TException {
    scheme(iprot).read(iprot, this);
  }

  public void write(org.apache.thrift.protocol.TProtocol oprot) throws org.apache.thrift.TException {
    scheme(oprot).write(oprot, this);
  }

  @Override
  public java.lang.String toString() {
    java.lang.StringBuilder sb = new java.lang.StringBuilder("SpecTimestamp(");
    boolean first = true;

    sb.append("comp:");
    if (this.comp == null) {
      sb.append("null");
    } else {
      sb.append(this.comp);
    }
    first = false;
    if (!first) sb.append(", ");
    sb.append("ts:");
    sb.append(this.ts);
    first = false;
    sb.append(")");
    return sb.toString();
  }

  public void validate() throws org.apache.thrift.TException {
    // check for required fields
    // check for sub-struct validity
  }

  private void writeObject(java.io.ObjectOutputStream out) throws java.io.IOException {
    try {
      write(new org.apache.thrift.protocol.TCompactProtocol(new org.apache.thrift.transport.TIOStreamTransport(out)));
    } catch (org.apache.thrift.TException te) {
      throw new java.io.IOException(te);
    }
  }

  private void readObject(java.io.ObjectInputStream in) throws java.io.IOException, java.lang.ClassNotFoundException {
    try {
      // it doesn't seem like you should have to do this, but java serialization is wacky, and doesn't call the default constructor.
      __isset_bitfield = 0;
      read(new org.apache.thrift.protocol.TCompactProtocol(new org.apache.thrift.transport.TIOStreamTransport(in)));
    } catch (org.apache.thrift.TException te) {
      throw new java.io.IOException(te);
    }
  }

  private static class SpecTimestampStandardSchemeFactory implements org.apache.thrift.scheme.SchemeFactory {
    public SpecTimestampStandardScheme getScheme() {
      return new SpecTimestampStandardScheme();
    }
  }

  private static class SpecTimestampStandardScheme extends org.apache.thrift.scheme.StandardScheme<SpecTimestamp> {

    public void read(org.apache.thrift.protocol.TProtocol iprot, SpecTimestamp struct) throws org.apache.thrift.TException {
      org.apache.thrift.protocol.TField schemeField;
      iprot.readStructBegin();
      while (true)
      {
        schemeField = iprot.readFieldBegin();
        if (schemeField.type == org.apache.thrift.protocol.TType.STOP) { 
          break;
        }
        switch (schemeField.id) {
          case 1: // COMP
            if (schemeField.type == org.apache.thrift.protocol.TType.I32) {
              struct.comp = org.swcdb.thrift.gen.Comp.findByValue(iprot.readI32());
              struct.setCompIsSet(true);
            } else { 
              org.apache.thrift.protocol.TProtocolUtil.skip(iprot, schemeField.type);
            }
            break;
          case 2: // TS
            if (schemeField.type == org.apache.thrift.protocol.TType.I64) {
              struct.ts = iprot.readI64();
              struct.setTsIsSet(true);
            } else { 
              org.apache.thrift.protocol.TProtocolUtil.skip(iprot, schemeField.type);
            }
            break;
          default:
            org.apache.thrift.protocol.TProtocolUtil.skip(iprot, schemeField.type);
        }
        iprot.readFieldEnd();
      }
      iprot.readStructEnd();

      // check for required fields of primitive type, which can't be checked in the validate method
      struct.validate();
    }

    public void write(org.apache.thrift.protocol.TProtocol oprot, SpecTimestamp struct) throws org.apache.thrift.TException {
      struct.validate();

      oprot.writeStructBegin(STRUCT_DESC);
      if (struct.comp != null) {
        oprot.writeFieldBegin(COMP_FIELD_DESC);
        oprot.writeI32(struct.comp.getValue());
        oprot.writeFieldEnd();
      }
      oprot.writeFieldBegin(TS_FIELD_DESC);
      oprot.writeI64(struct.ts);
      oprot.writeFieldEnd();
      oprot.writeFieldStop();
      oprot.writeStructEnd();
    }

  }

  private static class SpecTimestampTupleSchemeFactory implements org.apache.thrift.scheme.SchemeFactory {
    public SpecTimestampTupleScheme getScheme() {
      return new SpecTimestampTupleScheme();
    }
  }

  private static class SpecTimestampTupleScheme extends org.apache.thrift.scheme.TupleScheme<SpecTimestamp> {

    @Override
    public void write(org.apache.thrift.protocol.TProtocol prot, SpecTimestamp struct) throws org.apache.thrift.TException {
      org.apache.thrift.protocol.TTupleProtocol oprot = (org.apache.thrift.protocol.TTupleProtocol) prot;
      java.util.BitSet optionals = new java.util.BitSet();
      if (struct.isSetComp()) {
        optionals.set(0);
      }
      if (struct.isSetTs()) {
        optionals.set(1);
      }
      oprot.writeBitSet(optionals, 2);
      if (struct.isSetComp()) {
        oprot.writeI32(struct.comp.getValue());
      }
      if (struct.isSetTs()) {
        oprot.writeI64(struct.ts);
      }
    }

    @Override
    public void read(org.apache.thrift.protocol.TProtocol prot, SpecTimestamp struct) throws org.apache.thrift.TException {
      org.apache.thrift.protocol.TTupleProtocol iprot = (org.apache.thrift.protocol.TTupleProtocol) prot;
      java.util.BitSet incoming = iprot.readBitSet(2);
      if (incoming.get(0)) {
        struct.comp = org.swcdb.thrift.gen.Comp.findByValue(iprot.readI32());
        struct.setCompIsSet(true);
      }
      if (incoming.get(1)) {
        struct.ts = iprot.readI64();
        struct.setTsIsSet(true);
      }
    }
  }

  private static <S extends org.apache.thrift.scheme.IScheme> S scheme(org.apache.thrift.protocol.TProtocol proto) {
    return (org.apache.thrift.scheme.StandardScheme.class.equals(proto.getScheme()) ? STANDARD_SCHEME_FACTORY : TUPLE_SCHEME_FACTORY).getScheme();
  }
}

