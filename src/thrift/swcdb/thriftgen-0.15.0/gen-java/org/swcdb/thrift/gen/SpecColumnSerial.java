/**
 * Autogenerated by Thrift Compiler (0.15.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
package org.swcdb.thrift.gen;

@SuppressWarnings({"cast", "rawtypes", "serial", "unchecked", "unused"})
/**
 * The Column Specifications, the Cells-Intervals(SpecInterval/s) specification for a SERIAL Type Column
 */
public class SpecColumnSerial implements org.apache.thrift.TBase<SpecColumnSerial, SpecColumnSerial._Fields>, java.io.Serializable, Cloneable, Comparable<SpecColumnSerial> {
  private static final org.apache.thrift.protocol.TStruct STRUCT_DESC = new org.apache.thrift.protocol.TStruct("SpecColumnSerial");

  private static final org.apache.thrift.protocol.TField CID_FIELD_DESC = new org.apache.thrift.protocol.TField("cid", org.apache.thrift.protocol.TType.I64, (short)1);
  private static final org.apache.thrift.protocol.TField INTERVALS_FIELD_DESC = new org.apache.thrift.protocol.TField("intervals", org.apache.thrift.protocol.TType.LIST, (short)2);

  private static final org.apache.thrift.scheme.SchemeFactory STANDARD_SCHEME_FACTORY = new SpecColumnSerialStandardSchemeFactory();
  private static final org.apache.thrift.scheme.SchemeFactory TUPLE_SCHEME_FACTORY = new SpecColumnSerialTupleSchemeFactory();

  /**
   * The Column ID
   */
  public long cid; // required
  /**
   * The Serial Cells Interval in a list-container
   */
  public @org.apache.thrift.annotation.Nullable java.util.List<SpecIntervalSerial> intervals; // required

  /** The set of fields this struct contains, along with convenience methods for finding and manipulating them. */
  public enum _Fields implements org.apache.thrift.TFieldIdEnum {
    /**
     * The Column ID
     */
    CID((short)1, "cid"),
    /**
     * The Serial Cells Interval in a list-container
     */
    INTERVALS((short)2, "intervals");

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
        case 1: // CID
          return CID;
        case 2: // INTERVALS
          return INTERVALS;
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
  private static final int __CID_ISSET_ID = 0;
  private byte __isset_bitfield = 0;
  public static final java.util.Map<_Fields, org.apache.thrift.meta_data.FieldMetaData> metaDataMap;
  static {
    java.util.Map<_Fields, org.apache.thrift.meta_data.FieldMetaData> tmpMap = new java.util.EnumMap<_Fields, org.apache.thrift.meta_data.FieldMetaData>(_Fields.class);
    tmpMap.put(_Fields.CID, new org.apache.thrift.meta_data.FieldMetaData("cid", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.I64)));
    tmpMap.put(_Fields.INTERVALS, new org.apache.thrift.meta_data.FieldMetaData("intervals", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.ListMetaData(org.apache.thrift.protocol.TType.LIST, 
            new org.apache.thrift.meta_data.StructMetaData(org.apache.thrift.protocol.TType.STRUCT, SpecIntervalSerial.class))));
    metaDataMap = java.util.Collections.unmodifiableMap(tmpMap);
    org.apache.thrift.meta_data.FieldMetaData.addStructMetaDataMap(SpecColumnSerial.class, metaDataMap);
  }

  public SpecColumnSerial() {
  }

  public SpecColumnSerial(
    long cid,
    java.util.List<SpecIntervalSerial> intervals)
  {
    this();
    this.cid = cid;
    setCidIsSet(true);
    this.intervals = intervals;
  }

  /**
   * Performs a deep copy on <i>other</i>.
   */
  public SpecColumnSerial(SpecColumnSerial other) {
    __isset_bitfield = other.__isset_bitfield;
    this.cid = other.cid;
    if (other.isSetIntervals()) {
      java.util.List<SpecIntervalSerial> __this__intervals = new java.util.ArrayList<SpecIntervalSerial>(other.intervals.size());
      for (SpecIntervalSerial other_element : other.intervals) {
        __this__intervals.add(new SpecIntervalSerial(other_element));
      }
      this.intervals = __this__intervals;
    }
  }

  public SpecColumnSerial deepCopy() {
    return new SpecColumnSerial(this);
  }

  @Override
  public void clear() {
    setCidIsSet(false);
    this.cid = 0;
    this.intervals = null;
  }

  /**
   * The Column ID
   */
  public long getCid() {
    return this.cid;
  }

  /**
   * The Column ID
   */
  public SpecColumnSerial setCid(long cid) {
    this.cid = cid;
    setCidIsSet(true);
    return this;
  }

  public void unsetCid() {
    __isset_bitfield = org.apache.thrift.EncodingUtils.clearBit(__isset_bitfield, __CID_ISSET_ID);
  }

  /** Returns true if field cid is set (has been assigned a value) and false otherwise */
  public boolean isSetCid() {
    return org.apache.thrift.EncodingUtils.testBit(__isset_bitfield, __CID_ISSET_ID);
  }

  public void setCidIsSet(boolean value) {
    __isset_bitfield = org.apache.thrift.EncodingUtils.setBit(__isset_bitfield, __CID_ISSET_ID, value);
  }

  public int getIntervalsSize() {
    return (this.intervals == null) ? 0 : this.intervals.size();
  }

  @org.apache.thrift.annotation.Nullable
  public java.util.Iterator<SpecIntervalSerial> getIntervalsIterator() {
    return (this.intervals == null) ? null : this.intervals.iterator();
  }

  public void addToIntervals(SpecIntervalSerial elem) {
    if (this.intervals == null) {
      this.intervals = new java.util.ArrayList<SpecIntervalSerial>();
    }
    this.intervals.add(elem);
  }

  /**
   * The Serial Cells Interval in a list-container
   */
  @org.apache.thrift.annotation.Nullable
  public java.util.List<SpecIntervalSerial> getIntervals() {
    return this.intervals;
  }

  /**
   * The Serial Cells Interval in a list-container
   */
  public SpecColumnSerial setIntervals(@org.apache.thrift.annotation.Nullable java.util.List<SpecIntervalSerial> intervals) {
    this.intervals = intervals;
    return this;
  }

  public void unsetIntervals() {
    this.intervals = null;
  }

  /** Returns true if field intervals is set (has been assigned a value) and false otherwise */
  public boolean isSetIntervals() {
    return this.intervals != null;
  }

  public void setIntervalsIsSet(boolean value) {
    if (!value) {
      this.intervals = null;
    }
  }

  public void setFieldValue(_Fields field, @org.apache.thrift.annotation.Nullable java.lang.Object value) {
    switch (field) {
    case CID:
      if (value == null) {
        unsetCid();
      } else {
        setCid((java.lang.Long)value);
      }
      break;

    case INTERVALS:
      if (value == null) {
        unsetIntervals();
      } else {
        setIntervals((java.util.List<SpecIntervalSerial>)value);
      }
      break;

    }
  }

  @org.apache.thrift.annotation.Nullable
  public java.lang.Object getFieldValue(_Fields field) {
    switch (field) {
    case CID:
      return getCid();

    case INTERVALS:
      return getIntervals();

    }
    throw new java.lang.IllegalStateException();
  }

  /** Returns true if field corresponding to fieldID is set (has been assigned a value) and false otherwise */
  public boolean isSet(_Fields field) {
    if (field == null) {
      throw new java.lang.IllegalArgumentException();
    }

    switch (field) {
    case CID:
      return isSetCid();
    case INTERVALS:
      return isSetIntervals();
    }
    throw new java.lang.IllegalStateException();
  }

  @Override
  public boolean equals(java.lang.Object that) {
    if (that instanceof SpecColumnSerial)
      return this.equals((SpecColumnSerial)that);
    return false;
  }

  public boolean equals(SpecColumnSerial that) {
    if (that == null)
      return false;
    if (this == that)
      return true;

    boolean this_present_cid = true;
    boolean that_present_cid = true;
    if (this_present_cid || that_present_cid) {
      if (!(this_present_cid && that_present_cid))
        return false;
      if (this.cid != that.cid)
        return false;
    }

    boolean this_present_intervals = true && this.isSetIntervals();
    boolean that_present_intervals = true && that.isSetIntervals();
    if (this_present_intervals || that_present_intervals) {
      if (!(this_present_intervals && that_present_intervals))
        return false;
      if (!this.intervals.equals(that.intervals))
        return false;
    }

    return true;
  }

  @Override
  public int hashCode() {
    int hashCode = 1;

    hashCode = hashCode * 8191 + org.apache.thrift.TBaseHelper.hashCode(cid);

    hashCode = hashCode * 8191 + ((isSetIntervals()) ? 131071 : 524287);
    if (isSetIntervals())
      hashCode = hashCode * 8191 + intervals.hashCode();

    return hashCode;
  }

  @Override
  public int compareTo(SpecColumnSerial other) {
    if (!getClass().equals(other.getClass())) {
      return getClass().getName().compareTo(other.getClass().getName());
    }

    int lastComparison = 0;

    lastComparison = java.lang.Boolean.compare(isSetCid(), other.isSetCid());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetCid()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.cid, other.cid);
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    lastComparison = java.lang.Boolean.compare(isSetIntervals(), other.isSetIntervals());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetIntervals()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.intervals, other.intervals);
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
    java.lang.StringBuilder sb = new java.lang.StringBuilder("SpecColumnSerial(");
    boolean first = true;

    sb.append("cid:");
    sb.append(this.cid);
    first = false;
    if (!first) sb.append(", ");
    sb.append("intervals:");
    if (this.intervals == null) {
      sb.append("null");
    } else {
      sb.append(this.intervals);
    }
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

  private static class SpecColumnSerialStandardSchemeFactory implements org.apache.thrift.scheme.SchemeFactory {
    public SpecColumnSerialStandardScheme getScheme() {
      return new SpecColumnSerialStandardScheme();
    }
  }

  private static class SpecColumnSerialStandardScheme extends org.apache.thrift.scheme.StandardScheme<SpecColumnSerial> {

    public void read(org.apache.thrift.protocol.TProtocol iprot, SpecColumnSerial struct) throws org.apache.thrift.TException {
      org.apache.thrift.protocol.TField schemeField;
      iprot.readStructBegin();
      while (true)
      {
        schemeField = iprot.readFieldBegin();
        if (schemeField.type == org.apache.thrift.protocol.TType.STOP) { 
          break;
        }
        switch (schemeField.id) {
          case 1: // CID
            if (schemeField.type == org.apache.thrift.protocol.TType.I64) {
              struct.cid = iprot.readI64();
              struct.setCidIsSet(true);
            } else { 
              org.apache.thrift.protocol.TProtocolUtil.skip(iprot, schemeField.type);
            }
            break;
          case 2: // INTERVALS
            if (schemeField.type == org.apache.thrift.protocol.TType.LIST) {
              {
                org.apache.thrift.protocol.TList _list176 = iprot.readListBegin();
                struct.intervals = new java.util.ArrayList<SpecIntervalSerial>(_list176.size);
                @org.apache.thrift.annotation.Nullable SpecIntervalSerial _elem177;
                for (int _i178 = 0; _i178 < _list176.size; ++_i178)
                {
                  _elem177 = new SpecIntervalSerial();
                  _elem177.read(iprot);
                  struct.intervals.add(_elem177);
                }
                iprot.readListEnd();
              }
              struct.setIntervalsIsSet(true);
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

    public void write(org.apache.thrift.protocol.TProtocol oprot, SpecColumnSerial struct) throws org.apache.thrift.TException {
      struct.validate();

      oprot.writeStructBegin(STRUCT_DESC);
      oprot.writeFieldBegin(CID_FIELD_DESC);
      oprot.writeI64(struct.cid);
      oprot.writeFieldEnd();
      if (struct.intervals != null) {
        oprot.writeFieldBegin(INTERVALS_FIELD_DESC);
        {
          oprot.writeListBegin(new org.apache.thrift.protocol.TList(org.apache.thrift.protocol.TType.STRUCT, struct.intervals.size()));
          for (SpecIntervalSerial _iter179 : struct.intervals)
          {
            _iter179.write(oprot);
          }
          oprot.writeListEnd();
        }
        oprot.writeFieldEnd();
      }
      oprot.writeFieldStop();
      oprot.writeStructEnd();
    }

  }

  private static class SpecColumnSerialTupleSchemeFactory implements org.apache.thrift.scheme.SchemeFactory {
    public SpecColumnSerialTupleScheme getScheme() {
      return new SpecColumnSerialTupleScheme();
    }
  }

  private static class SpecColumnSerialTupleScheme extends org.apache.thrift.scheme.TupleScheme<SpecColumnSerial> {

    @Override
    public void write(org.apache.thrift.protocol.TProtocol prot, SpecColumnSerial struct) throws org.apache.thrift.TException {
      org.apache.thrift.protocol.TTupleProtocol oprot = (org.apache.thrift.protocol.TTupleProtocol) prot;
      java.util.BitSet optionals = new java.util.BitSet();
      if (struct.isSetCid()) {
        optionals.set(0);
      }
      if (struct.isSetIntervals()) {
        optionals.set(1);
      }
      oprot.writeBitSet(optionals, 2);
      if (struct.isSetCid()) {
        oprot.writeI64(struct.cid);
      }
      if (struct.isSetIntervals()) {
        {
          oprot.writeI32(struct.intervals.size());
          for (SpecIntervalSerial _iter180 : struct.intervals)
          {
            _iter180.write(oprot);
          }
        }
      }
    }

    @Override
    public void read(org.apache.thrift.protocol.TProtocol prot, SpecColumnSerial struct) throws org.apache.thrift.TException {
      org.apache.thrift.protocol.TTupleProtocol iprot = (org.apache.thrift.protocol.TTupleProtocol) prot;
      java.util.BitSet incoming = iprot.readBitSet(2);
      if (incoming.get(0)) {
        struct.cid = iprot.readI64();
        struct.setCidIsSet(true);
      }
      if (incoming.get(1)) {
        {
          org.apache.thrift.protocol.TList _list181 = iprot.readListBegin(org.apache.thrift.protocol.TType.STRUCT);
          struct.intervals = new java.util.ArrayList<SpecIntervalSerial>(_list181.size);
          @org.apache.thrift.annotation.Nullable SpecIntervalSerial _elem182;
          for (int _i183 = 0; _i183 < _list181.size; ++_i183)
          {
            _elem182 = new SpecIntervalSerial();
            _elem182.read(iprot);
            struct.intervals.add(_elem182);
          }
        }
        struct.setIntervalsIsSet(true);
      }
    }
  }

  private static <S extends org.apache.thrift.scheme.IScheme> S scheme(org.apache.thrift.protocol.TProtocol proto) {
    return (org.apache.thrift.scheme.StandardScheme.class.equals(proto.getScheme()) ? STANDARD_SCHEME_FACTORY : TUPLE_SCHEME_FACTORY).getScheme();
  }
}
