/**
 * Autogenerated by Thrift Compiler (0.15.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
package org.swcdb.thrift.gen;

@SuppressWarnings({"cast", "rawtypes", "serial", "unchecked", "unused"})
/**
 * The Serial Cell for results list of scan
 */
public class CellSerial implements org.apache.thrift.TBase<CellSerial, CellSerial._Fields>, java.io.Serializable, Cloneable, Comparable<CellSerial> {
  private static final org.apache.thrift.protocol.TStruct STRUCT_DESC = new org.apache.thrift.protocol.TStruct("CellSerial");

  private static final org.apache.thrift.protocol.TField C_FIELD_DESC = new org.apache.thrift.protocol.TField("c", org.apache.thrift.protocol.TType.STRING, (short)1);
  private static final org.apache.thrift.protocol.TField K_FIELD_DESC = new org.apache.thrift.protocol.TField("k", org.apache.thrift.protocol.TType.LIST, (short)2);
  private static final org.apache.thrift.protocol.TField TS_FIELD_DESC = new org.apache.thrift.protocol.TField("ts", org.apache.thrift.protocol.TType.I64, (short)3);
  private static final org.apache.thrift.protocol.TField V_FIELD_DESC = new org.apache.thrift.protocol.TField("v", org.apache.thrift.protocol.TType.LIST, (short)4);

  private static final org.apache.thrift.scheme.SchemeFactory STANDARD_SCHEME_FACTORY = new CellSerialStandardSchemeFactory();
  private static final org.apache.thrift.scheme.SchemeFactory TUPLE_SCHEME_FACTORY = new CellSerialTupleSchemeFactory();

  /**
   * The Column Name
   */
  public @org.apache.thrift.annotation.Nullable java.lang.String c; // required
  /**
   * The Cell Key
   */
  public @org.apache.thrift.annotation.Nullable java.util.List<java.nio.ByteBuffer> k; // required
  /**
   * The Cell Timestamp
   */
  public long ts; // required
  /**
   * The Cell Serial Value
   */
  public @org.apache.thrift.annotation.Nullable java.util.List<CellValueSerial> v; // required

  /** The set of fields this struct contains, along with convenience methods for finding and manipulating them. */
  public enum _Fields implements org.apache.thrift.TFieldIdEnum {
    /**
     * The Column Name
     */
    C((short)1, "c"),
    /**
     * The Cell Key
     */
    K((short)2, "k"),
    /**
     * The Cell Timestamp
     */
    TS((short)3, "ts"),
    /**
     * The Cell Serial Value
     */
    V((short)4, "v");

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
        case 1: // C
          return C;
        case 2: // K
          return K;
        case 3: // TS
          return TS;
        case 4: // V
          return V;
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
    tmpMap.put(_Fields.C, new org.apache.thrift.meta_data.FieldMetaData("c", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.STRING)));
    tmpMap.put(_Fields.K, new org.apache.thrift.meta_data.FieldMetaData("k", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.LIST        , "Key")));
    tmpMap.put(_Fields.TS, new org.apache.thrift.meta_data.FieldMetaData("ts", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.I64)));
    tmpMap.put(_Fields.V, new org.apache.thrift.meta_data.FieldMetaData("v", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.LIST        , "CellValuesSerial")));
    metaDataMap = java.util.Collections.unmodifiableMap(tmpMap);
    org.apache.thrift.meta_data.FieldMetaData.addStructMetaDataMap(CellSerial.class, metaDataMap);
  }

  public CellSerial() {
  }

  public CellSerial(
    java.lang.String c,
    java.util.List<java.nio.ByteBuffer> k,
    long ts,
    java.util.List<CellValueSerial> v)
  {
    this();
    this.c = c;
    this.k = k;
    this.ts = ts;
    setTsIsSet(true);
    this.v = v;
  }

  /**
   * Performs a deep copy on <i>other</i>.
   */
  public CellSerial(CellSerial other) {
    __isset_bitfield = other.__isset_bitfield;
    if (other.isSetC()) {
      this.c = other.c;
    }
    if (other.isSetK()) {
      java.util.List<java.nio.ByteBuffer> __this__k = new java.util.ArrayList<java.nio.ByteBuffer>(other.k);
      this.k = __this__k;
    }
    this.ts = other.ts;
    if (other.isSetV()) {
      java.util.List<CellValueSerial> __this__v = new java.util.ArrayList<CellValueSerial>(other.v.size());
      for (CellValueSerial other_element : other.v) {
        __this__v.add(new CellValueSerial(other_element));
      }
      this.v = __this__v;
    }
  }

  public CellSerial deepCopy() {
    return new CellSerial(this);
  }

  @Override
  public void clear() {
    this.c = null;
    this.k = null;
    setTsIsSet(false);
    this.ts = 0;
    this.v = null;
  }

  /**
   * The Column Name
   */
  @org.apache.thrift.annotation.Nullable
  public java.lang.String getC() {
    return this.c;
  }

  /**
   * The Column Name
   */
  public CellSerial setC(@org.apache.thrift.annotation.Nullable java.lang.String c) {
    this.c = c;
    return this;
  }

  public void unsetC() {
    this.c = null;
  }

  /** Returns true if field c is set (has been assigned a value) and false otherwise */
  public boolean isSetC() {
    return this.c != null;
  }

  public void setCIsSet(boolean value) {
    if (!value) {
      this.c = null;
    }
  }

  public int getKSize() {
    return (this.k == null) ? 0 : this.k.size();
  }

  @org.apache.thrift.annotation.Nullable
  public java.util.Iterator<java.nio.ByteBuffer> getKIterator() {
    return (this.k == null) ? null : this.k.iterator();
  }

  public void addToK(java.nio.ByteBuffer elem) {
    if (this.k == null) {
      this.k = new java.util.ArrayList<java.nio.ByteBuffer>();
    }
    this.k.add(elem);
  }

  /**
   * The Cell Key
   */
  @org.apache.thrift.annotation.Nullable
  public java.util.List<java.nio.ByteBuffer> getK() {
    return this.k;
  }

  /**
   * The Cell Key
   */
  public CellSerial setK(@org.apache.thrift.annotation.Nullable java.util.List<java.nio.ByteBuffer> k) {
    this.k = k;
    return this;
  }

  public void unsetK() {
    this.k = null;
  }

  /** Returns true if field k is set (has been assigned a value) and false otherwise */
  public boolean isSetK() {
    return this.k != null;
  }

  public void setKIsSet(boolean value) {
    if (!value) {
      this.k = null;
    }
  }

  /**
   * The Cell Timestamp
   */
  public long getTs() {
    return this.ts;
  }

  /**
   * The Cell Timestamp
   */
  public CellSerial setTs(long ts) {
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

  public int getVSize() {
    return (this.v == null) ? 0 : this.v.size();
  }

  @org.apache.thrift.annotation.Nullable
  public java.util.Iterator<CellValueSerial> getVIterator() {
    return (this.v == null) ? null : this.v.iterator();
  }

  public void addToV(CellValueSerial elem) {
    if (this.v == null) {
      this.v = new java.util.ArrayList<CellValueSerial>();
    }
    this.v.add(elem);
  }

  /**
   * The Cell Serial Value
   */
  @org.apache.thrift.annotation.Nullable
  public java.util.List<CellValueSerial> getV() {
    return this.v;
  }

  /**
   * The Cell Serial Value
   */
  public CellSerial setV(@org.apache.thrift.annotation.Nullable java.util.List<CellValueSerial> v) {
    this.v = v;
    return this;
  }

  public void unsetV() {
    this.v = null;
  }

  /** Returns true if field v is set (has been assigned a value) and false otherwise */
  public boolean isSetV() {
    return this.v != null;
  }

  public void setVIsSet(boolean value) {
    if (!value) {
      this.v = null;
    }
  }

  public void setFieldValue(_Fields field, @org.apache.thrift.annotation.Nullable java.lang.Object value) {
    switch (field) {
    case C:
      if (value == null) {
        unsetC();
      } else {
        setC((java.lang.String)value);
      }
      break;

    case K:
      if (value == null) {
        unsetK();
      } else {
        setK((java.util.List<java.nio.ByteBuffer>)value);
      }
      break;

    case TS:
      if (value == null) {
        unsetTs();
      } else {
        setTs((java.lang.Long)value);
      }
      break;

    case V:
      if (value == null) {
        unsetV();
      } else {
        setV((java.util.List<CellValueSerial>)value);
      }
      break;

    }
  }

  @org.apache.thrift.annotation.Nullable
  public java.lang.Object getFieldValue(_Fields field) {
    switch (field) {
    case C:
      return getC();

    case K:
      return getK();

    case TS:
      return getTs();

    case V:
      return getV();

    }
    throw new java.lang.IllegalStateException();
  }

  /** Returns true if field corresponding to fieldID is set (has been assigned a value) and false otherwise */
  public boolean isSet(_Fields field) {
    if (field == null) {
      throw new java.lang.IllegalArgumentException();
    }

    switch (field) {
    case C:
      return isSetC();
    case K:
      return isSetK();
    case TS:
      return isSetTs();
    case V:
      return isSetV();
    }
    throw new java.lang.IllegalStateException();
  }

  @Override
  public boolean equals(java.lang.Object that) {
    if (that instanceof CellSerial)
      return this.equals((CellSerial)that);
    return false;
  }

  public boolean equals(CellSerial that) {
    if (that == null)
      return false;
    if (this == that)
      return true;

    boolean this_present_c = true && this.isSetC();
    boolean that_present_c = true && that.isSetC();
    if (this_present_c || that_present_c) {
      if (!(this_present_c && that_present_c))
        return false;
      if (!this.c.equals(that.c))
        return false;
    }

    boolean this_present_k = true && this.isSetK();
    boolean that_present_k = true && that.isSetK();
    if (this_present_k || that_present_k) {
      if (!(this_present_k && that_present_k))
        return false;
      if (!this.k.equals(that.k))
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

    boolean this_present_v = true && this.isSetV();
    boolean that_present_v = true && that.isSetV();
    if (this_present_v || that_present_v) {
      if (!(this_present_v && that_present_v))
        return false;
      if (!this.v.equals(that.v))
        return false;
    }

    return true;
  }

  @Override
  public int hashCode() {
    int hashCode = 1;

    hashCode = hashCode * 8191 + ((isSetC()) ? 131071 : 524287);
    if (isSetC())
      hashCode = hashCode * 8191 + c.hashCode();

    hashCode = hashCode * 8191 + ((isSetK()) ? 131071 : 524287);
    if (isSetK())
      hashCode = hashCode * 8191 + k.hashCode();

    hashCode = hashCode * 8191 + org.apache.thrift.TBaseHelper.hashCode(ts);

    hashCode = hashCode * 8191 + ((isSetV()) ? 131071 : 524287);
    if (isSetV())
      hashCode = hashCode * 8191 + v.hashCode();

    return hashCode;
  }

  @Override
  public int compareTo(CellSerial other) {
    if (!getClass().equals(other.getClass())) {
      return getClass().getName().compareTo(other.getClass().getName());
    }

    int lastComparison = 0;

    lastComparison = java.lang.Boolean.compare(isSetC(), other.isSetC());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetC()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.c, other.c);
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    lastComparison = java.lang.Boolean.compare(isSetK(), other.isSetK());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetK()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.k, other.k);
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    lastComparison = java.lang.Boolean.compare(isSetTs(), other.isSetTs());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetTs()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.ts, other.ts);
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    lastComparison = java.lang.Boolean.compare(isSetV(), other.isSetV());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetV()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.v, other.v);
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
    java.lang.StringBuilder sb = new java.lang.StringBuilder("CellSerial(");
    boolean first = true;

    sb.append("c:");
    if (this.c == null) {
      sb.append("null");
    } else {
      sb.append(this.c);
    }
    first = false;
    if (!first) sb.append(", ");
    sb.append("k:");
    if (this.k == null) {
      sb.append("null");
    } else {
      sb.append(this.k);
    }
    first = false;
    if (!first) sb.append(", ");
    sb.append("ts:");
    sb.append(this.ts);
    first = false;
    if (!first) sb.append(", ");
    sb.append("v:");
    if (this.v == null) {
      sb.append("null");
    } else {
      sb.append(this.v);
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

  private static class CellSerialStandardSchemeFactory implements org.apache.thrift.scheme.SchemeFactory {
    public CellSerialStandardScheme getScheme() {
      return new CellSerialStandardScheme();
    }
  }

  private static class CellSerialStandardScheme extends org.apache.thrift.scheme.StandardScheme<CellSerial> {

    public void read(org.apache.thrift.protocol.TProtocol iprot, CellSerial struct) throws org.apache.thrift.TException {
      org.apache.thrift.protocol.TField schemeField;
      iprot.readStructBegin();
      while (true)
      {
        schemeField = iprot.readFieldBegin();
        if (schemeField.type == org.apache.thrift.protocol.TType.STOP) { 
          break;
        }
        switch (schemeField.id) {
          case 1: // C
            if (schemeField.type == org.apache.thrift.protocol.TType.STRING) {
              struct.c = iprot.readString();
              struct.setCIsSet(true);
            } else { 
              org.apache.thrift.protocol.TProtocolUtil.skip(iprot, schemeField.type);
            }
            break;
          case 2: // K
            if (schemeField.type == org.apache.thrift.protocol.TType.LIST) {
              {
                org.apache.thrift.protocol.TList _list264 = iprot.readListBegin();
                struct.k = new java.util.ArrayList<java.nio.ByteBuffer>(_list264.size);
                @org.apache.thrift.annotation.Nullable java.nio.ByteBuffer _elem265;
                for (int _i266 = 0; _i266 < _list264.size; ++_i266)
                {
                  _elem265 = iprot.readBinary();
                  struct.k.add(_elem265);
                }
                iprot.readListEnd();
              }
              struct.setKIsSet(true);
            } else { 
              org.apache.thrift.protocol.TProtocolUtil.skip(iprot, schemeField.type);
            }
            break;
          case 3: // TS
            if (schemeField.type == org.apache.thrift.protocol.TType.I64) {
              struct.ts = iprot.readI64();
              struct.setTsIsSet(true);
            } else { 
              org.apache.thrift.protocol.TProtocolUtil.skip(iprot, schemeField.type);
            }
            break;
          case 4: // V
            if (schemeField.type == org.apache.thrift.protocol.TType.LIST) {
              {
                org.apache.thrift.protocol.TList _list267 = iprot.readListBegin();
                struct.v = new java.util.ArrayList<CellValueSerial>(_list267.size);
                @org.apache.thrift.annotation.Nullable CellValueSerial _elem268;
                for (int _i269 = 0; _i269 < _list267.size; ++_i269)
                {
                  _elem268 = new CellValueSerial();
                  _elem268.read(iprot);
                  struct.v.add(_elem268);
                }
                iprot.readListEnd();
              }
              struct.setVIsSet(true);
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

    public void write(org.apache.thrift.protocol.TProtocol oprot, CellSerial struct) throws org.apache.thrift.TException {
      struct.validate();

      oprot.writeStructBegin(STRUCT_DESC);
      if (struct.c != null) {
        oprot.writeFieldBegin(C_FIELD_DESC);
        oprot.writeString(struct.c);
        oprot.writeFieldEnd();
      }
      if (struct.k != null) {
        oprot.writeFieldBegin(K_FIELD_DESC);
        {
          oprot.writeListBegin(new org.apache.thrift.protocol.TList(org.apache.thrift.protocol.TType.STRING, struct.k.size()));
          for (java.nio.ByteBuffer _iter270 : struct.k)
          {
            oprot.writeBinary(_iter270);
          }
          oprot.writeListEnd();
        }
        oprot.writeFieldEnd();
      }
      oprot.writeFieldBegin(TS_FIELD_DESC);
      oprot.writeI64(struct.ts);
      oprot.writeFieldEnd();
      if (struct.v != null) {
        oprot.writeFieldBegin(V_FIELD_DESC);
        {
          oprot.writeListBegin(new org.apache.thrift.protocol.TList(org.apache.thrift.protocol.TType.STRUCT, struct.v.size()));
          for (CellValueSerial _iter271 : struct.v)
          {
            _iter271.write(oprot);
          }
          oprot.writeListEnd();
        }
        oprot.writeFieldEnd();
      }
      oprot.writeFieldStop();
      oprot.writeStructEnd();
    }

  }

  private static class CellSerialTupleSchemeFactory implements org.apache.thrift.scheme.SchemeFactory {
    public CellSerialTupleScheme getScheme() {
      return new CellSerialTupleScheme();
    }
  }

  private static class CellSerialTupleScheme extends org.apache.thrift.scheme.TupleScheme<CellSerial> {

    @Override
    public void write(org.apache.thrift.protocol.TProtocol prot, CellSerial struct) throws org.apache.thrift.TException {
      org.apache.thrift.protocol.TTupleProtocol oprot = (org.apache.thrift.protocol.TTupleProtocol) prot;
      java.util.BitSet optionals = new java.util.BitSet();
      if (struct.isSetC()) {
        optionals.set(0);
      }
      if (struct.isSetK()) {
        optionals.set(1);
      }
      if (struct.isSetTs()) {
        optionals.set(2);
      }
      if (struct.isSetV()) {
        optionals.set(3);
      }
      oprot.writeBitSet(optionals, 4);
      if (struct.isSetC()) {
        oprot.writeString(struct.c);
      }
      if (struct.isSetK()) {
        {
          oprot.writeI32(struct.k.size());
          for (java.nio.ByteBuffer _iter272 : struct.k)
          {
            oprot.writeBinary(_iter272);
          }
        }
      }
      if (struct.isSetTs()) {
        oprot.writeI64(struct.ts);
      }
      if (struct.isSetV()) {
        {
          oprot.writeI32(struct.v.size());
          for (CellValueSerial _iter273 : struct.v)
          {
            _iter273.write(oprot);
          }
        }
      }
    }

    @Override
    public void read(org.apache.thrift.protocol.TProtocol prot, CellSerial struct) throws org.apache.thrift.TException {
      org.apache.thrift.protocol.TTupleProtocol iprot = (org.apache.thrift.protocol.TTupleProtocol) prot;
      java.util.BitSet incoming = iprot.readBitSet(4);
      if (incoming.get(0)) {
        struct.c = iprot.readString();
        struct.setCIsSet(true);
      }
      if (incoming.get(1)) {
        {
          org.apache.thrift.protocol.TList _list274 = iprot.readListBegin(org.apache.thrift.protocol.TType.STRING);
          struct.k = new java.util.ArrayList<java.nio.ByteBuffer>(_list274.size);
          @org.apache.thrift.annotation.Nullable java.nio.ByteBuffer _elem275;
          for (int _i276 = 0; _i276 < _list274.size; ++_i276)
          {
            _elem275 = iprot.readBinary();
            struct.k.add(_elem275);
          }
        }
        struct.setKIsSet(true);
      }
      if (incoming.get(2)) {
        struct.ts = iprot.readI64();
        struct.setTsIsSet(true);
      }
      if (incoming.get(3)) {
        {
          org.apache.thrift.protocol.TList _list277 = iprot.readListBegin(org.apache.thrift.protocol.TType.STRUCT);
          struct.v = new java.util.ArrayList<CellValueSerial>(_list277.size);
          @org.apache.thrift.annotation.Nullable CellValueSerial _elem278;
          for (int _i279 = 0; _i279 < _list277.size; ++_i279)
          {
            _elem278 = new CellValueSerial();
            _elem278.read(iprot);
            struct.v.add(_elem278);
          }
        }
        struct.setVIsSet(true);
      }
    }
  }

  private static <S extends org.apache.thrift.scheme.IScheme> S scheme(org.apache.thrift.protocol.TProtocol proto) {
    return (org.apache.thrift.scheme.StandardScheme.class.equals(proto.getScheme()) ? STANDARD_SCHEME_FACTORY : TUPLE_SCHEME_FACTORY).getScheme();
  }
}

