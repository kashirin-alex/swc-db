/**
 * Autogenerated by Thrift Compiler (0.15.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
package org.swcdb.thrift.gen;

@SuppressWarnings({"cast", "rawtypes", "serial", "unchecked", "unused"})
/**
 * The Serial Value Specifications
 */
public class SpecValueSerial implements org.apache.thrift.TBase<SpecValueSerial, SpecValueSerial._Fields>, java.io.Serializable, Cloneable, Comparable<SpecValueSerial> {
  private static final org.apache.thrift.protocol.TStruct STRUCT_DESC = new org.apache.thrift.protocol.TStruct("SpecValueSerial");

  private static final org.apache.thrift.protocol.TField COMP_FIELD_DESC = new org.apache.thrift.protocol.TField("comp", org.apache.thrift.protocol.TType.I32, (short)1);
  private static final org.apache.thrift.protocol.TField FIELDS_FIELD_DESC = new org.apache.thrift.protocol.TField("fields", org.apache.thrift.protocol.TType.LIST, (short)2);

  private static final org.apache.thrift.scheme.SchemeFactory STANDARD_SCHEME_FACTORY = new SpecValueSerialStandardSchemeFactory();
  private static final org.apache.thrift.scheme.SchemeFactory TUPLE_SCHEME_FACTORY = new SpecValueSerialTupleSchemeFactory();

  /**
   * Logical comparator to Apply
   * 
   * @see Comp
   */
  public @org.apache.thrift.annotation.Nullable Comp comp; // required
  /**
   * The Serial Value Specifications to match against the SERIAL Cell value fields
   */
  public @org.apache.thrift.annotation.Nullable java.util.List<SpecValueSerialField> fields; // required

  /** The set of fields this struct contains, along with convenience methods for finding and manipulating them. */
  public enum _Fields implements org.apache.thrift.TFieldIdEnum {
    /**
     * Logical comparator to Apply
     * 
     * @see Comp
     */
    COMP((short)1, "comp"),
    /**
     * The Serial Value Specifications to match against the SERIAL Cell value fields
     */
    FIELDS((short)2, "fields");

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
        case 2: // FIELDS
          return FIELDS;
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
  public static final java.util.Map<_Fields, org.apache.thrift.meta_data.FieldMetaData> metaDataMap;
  static {
    java.util.Map<_Fields, org.apache.thrift.meta_data.FieldMetaData> tmpMap = new java.util.EnumMap<_Fields, org.apache.thrift.meta_data.FieldMetaData>(_Fields.class);
    tmpMap.put(_Fields.COMP, new org.apache.thrift.meta_data.FieldMetaData("comp", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.EnumMetaData(org.apache.thrift.protocol.TType.ENUM, Comp.class)));
    tmpMap.put(_Fields.FIELDS, new org.apache.thrift.meta_data.FieldMetaData("fields", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.LIST        , "SpecValueSerialFields")));
    metaDataMap = java.util.Collections.unmodifiableMap(tmpMap);
    org.apache.thrift.meta_data.FieldMetaData.addStructMetaDataMap(SpecValueSerial.class, metaDataMap);
  }

  public SpecValueSerial() {
  }

  public SpecValueSerial(
    Comp comp,
    java.util.List<SpecValueSerialField> fields)
  {
    this();
    this.comp = comp;
    this.fields = fields;
  }

  /**
   * Performs a deep copy on <i>other</i>.
   */
  public SpecValueSerial(SpecValueSerial other) {
    if (other.isSetComp()) {
      this.comp = other.comp;
    }
    if (other.isSetFields()) {
      java.util.List<SpecValueSerialField> __this__fields = new java.util.ArrayList<SpecValueSerialField>(other.fields.size());
      for (SpecValueSerialField other_element : other.fields) {
        __this__fields.add(new SpecValueSerialField(other_element));
      }
      this.fields = __this__fields;
    }
  }

  public SpecValueSerial deepCopy() {
    return new SpecValueSerial(this);
  }

  @Override
  public void clear() {
    this.comp = null;
    this.fields = null;
  }

  /**
   * Logical comparator to Apply
   * 
   * @see Comp
   */
  @org.apache.thrift.annotation.Nullable
  public Comp getComp() {
    return this.comp;
  }

  /**
   * Logical comparator to Apply
   * 
   * @see Comp
   */
  public SpecValueSerial setComp(@org.apache.thrift.annotation.Nullable Comp comp) {
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

  public int getFieldsSize() {
    return (this.fields == null) ? 0 : this.fields.size();
  }

  @org.apache.thrift.annotation.Nullable
  public java.util.Iterator<SpecValueSerialField> getFieldsIterator() {
    return (this.fields == null) ? null : this.fields.iterator();
  }

  public void addToFields(SpecValueSerialField elem) {
    if (this.fields == null) {
      this.fields = new java.util.ArrayList<SpecValueSerialField>();
    }
    this.fields.add(elem);
  }

  /**
   * The Serial Value Specifications to match against the SERIAL Cell value fields
   */
  @org.apache.thrift.annotation.Nullable
  public java.util.List<SpecValueSerialField> getFields() {
    return this.fields;
  }

  /**
   * The Serial Value Specifications to match against the SERIAL Cell value fields
   */
  public SpecValueSerial setFields(@org.apache.thrift.annotation.Nullable java.util.List<SpecValueSerialField> fields) {
    this.fields = fields;
    return this;
  }

  public void unsetFields() {
    this.fields = null;
  }

  /** Returns true if field fields is set (has been assigned a value) and false otherwise */
  public boolean isSetFields() {
    return this.fields != null;
  }

  public void setFieldsIsSet(boolean value) {
    if (!value) {
      this.fields = null;
    }
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

    case FIELDS:
      if (value == null) {
        unsetFields();
      } else {
        setFields((java.util.List<SpecValueSerialField>)value);
      }
      break;

    }
  }

  @org.apache.thrift.annotation.Nullable
  public java.lang.Object getFieldValue(_Fields field) {
    switch (field) {
    case COMP:
      return getComp();

    case FIELDS:
      return getFields();

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
    case FIELDS:
      return isSetFields();
    }
    throw new java.lang.IllegalStateException();
  }

  @Override
  public boolean equals(java.lang.Object that) {
    if (that instanceof SpecValueSerial)
      return this.equals((SpecValueSerial)that);
    return false;
  }

  public boolean equals(SpecValueSerial that) {
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

    boolean this_present_fields = true && this.isSetFields();
    boolean that_present_fields = true && that.isSetFields();
    if (this_present_fields || that_present_fields) {
      if (!(this_present_fields && that_present_fields))
        return false;
      if (!this.fields.equals(that.fields))
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

    hashCode = hashCode * 8191 + ((isSetFields()) ? 131071 : 524287);
    if (isSetFields())
      hashCode = hashCode * 8191 + fields.hashCode();

    return hashCode;
  }

  @Override
  public int compareTo(SpecValueSerial other) {
    if (!getClass().equals(other.getClass())) {
      return getClass().getName().compareTo(other.getClass().getName());
    }

    int lastComparison = 0;

    lastComparison = java.lang.Boolean.compare(isSetComp(), other.isSetComp());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetComp()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.comp, other.comp);
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    lastComparison = java.lang.Boolean.compare(isSetFields(), other.isSetFields());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetFields()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.fields, other.fields);
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
    java.lang.StringBuilder sb = new java.lang.StringBuilder("SpecValueSerial(");
    boolean first = true;

    sb.append("comp:");
    if (this.comp == null) {
      sb.append("null");
    } else {
      sb.append(this.comp);
    }
    first = false;
    if (!first) sb.append(", ");
    sb.append("fields:");
    if (this.fields == null) {
      sb.append("null");
    } else {
      sb.append(this.fields);
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
      read(new org.apache.thrift.protocol.TCompactProtocol(new org.apache.thrift.transport.TIOStreamTransport(in)));
    } catch (org.apache.thrift.TException te) {
      throw new java.io.IOException(te);
    }
  }

  private static class SpecValueSerialStandardSchemeFactory implements org.apache.thrift.scheme.SchemeFactory {
    public SpecValueSerialStandardScheme getScheme() {
      return new SpecValueSerialStandardScheme();
    }
  }

  private static class SpecValueSerialStandardScheme extends org.apache.thrift.scheme.StandardScheme<SpecValueSerial> {

    public void read(org.apache.thrift.protocol.TProtocol iprot, SpecValueSerial struct) throws org.apache.thrift.TException {
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
          case 2: // FIELDS
            if (schemeField.type == org.apache.thrift.protocol.TType.LIST) {
              {
                org.apache.thrift.protocol.TList _list136 = iprot.readListBegin();
                struct.fields = new java.util.ArrayList<SpecValueSerialField>(_list136.size);
                @org.apache.thrift.annotation.Nullable SpecValueSerialField _elem137;
                for (int _i138 = 0; _i138 < _list136.size; ++_i138)
                {
                  _elem137 = new SpecValueSerialField();
                  _elem137.read(iprot);
                  struct.fields.add(_elem137);
                }
                iprot.readListEnd();
              }
              struct.setFieldsIsSet(true);
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

    public void write(org.apache.thrift.protocol.TProtocol oprot, SpecValueSerial struct) throws org.apache.thrift.TException {
      struct.validate();

      oprot.writeStructBegin(STRUCT_DESC);
      if (struct.comp != null) {
        oprot.writeFieldBegin(COMP_FIELD_DESC);
        oprot.writeI32(struct.comp.getValue());
        oprot.writeFieldEnd();
      }
      if (struct.fields != null) {
        oprot.writeFieldBegin(FIELDS_FIELD_DESC);
        {
          oprot.writeListBegin(new org.apache.thrift.protocol.TList(org.apache.thrift.protocol.TType.STRUCT, struct.fields.size()));
          for (SpecValueSerialField _iter139 : struct.fields)
          {
            _iter139.write(oprot);
          }
          oprot.writeListEnd();
        }
        oprot.writeFieldEnd();
      }
      oprot.writeFieldStop();
      oprot.writeStructEnd();
    }

  }

  private static class SpecValueSerialTupleSchemeFactory implements org.apache.thrift.scheme.SchemeFactory {
    public SpecValueSerialTupleScheme getScheme() {
      return new SpecValueSerialTupleScheme();
    }
  }

  private static class SpecValueSerialTupleScheme extends org.apache.thrift.scheme.TupleScheme<SpecValueSerial> {

    @Override
    public void write(org.apache.thrift.protocol.TProtocol prot, SpecValueSerial struct) throws org.apache.thrift.TException {
      org.apache.thrift.protocol.TTupleProtocol oprot = (org.apache.thrift.protocol.TTupleProtocol) prot;
      java.util.BitSet optionals = new java.util.BitSet();
      if (struct.isSetComp()) {
        optionals.set(0);
      }
      if (struct.isSetFields()) {
        optionals.set(1);
      }
      oprot.writeBitSet(optionals, 2);
      if (struct.isSetComp()) {
        oprot.writeI32(struct.comp.getValue());
      }
      if (struct.isSetFields()) {
        {
          oprot.writeI32(struct.fields.size());
          for (SpecValueSerialField _iter140 : struct.fields)
          {
            _iter140.write(oprot);
          }
        }
      }
    }

    @Override
    public void read(org.apache.thrift.protocol.TProtocol prot, SpecValueSerial struct) throws org.apache.thrift.TException {
      org.apache.thrift.protocol.TTupleProtocol iprot = (org.apache.thrift.protocol.TTupleProtocol) prot;
      java.util.BitSet incoming = iprot.readBitSet(2);
      if (incoming.get(0)) {
        struct.comp = org.swcdb.thrift.gen.Comp.findByValue(iprot.readI32());
        struct.setCompIsSet(true);
      }
      if (incoming.get(1)) {
        {
          org.apache.thrift.protocol.TList _list141 = iprot.readListBegin(org.apache.thrift.protocol.TType.STRUCT);
          struct.fields = new java.util.ArrayList<SpecValueSerialField>(_list141.size);
          @org.apache.thrift.annotation.Nullable SpecValueSerialField _elem142;
          for (int _i143 = 0; _i143 < _list141.size; ++_i143)
          {
            _elem142 = new SpecValueSerialField();
            _elem142.read(iprot);
            struct.fields.add(_elem142);
          }
        }
        struct.setFieldsIsSet(true);
      }
    }
  }

  private static <S extends org.apache.thrift.scheme.IScheme> S scheme(org.apache.thrift.protocol.TProtocol proto) {
    return (org.apache.thrift.scheme.StandardScheme.class.equals(proto.getScheme()) ? STANDARD_SCHEME_FACTORY : TUPLE_SCHEME_FACTORY).getScheme();
  }
}

