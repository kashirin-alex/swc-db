/**
 * Autogenerated by Thrift Compiler (0.13.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
package org.swcdb.thrift.gen;

@SuppressWarnings({"cast", "rawtypes", "serial", "unchecked", "unused"})
public class FCells implements org.apache.thrift.TBase<FCells, FCells._Fields>, java.io.Serializable, Cloneable, Comparable<FCells> {
  private static final org.apache.thrift.protocol.TStruct STRUCT_DESC = new org.apache.thrift.protocol.TStruct("FCells");

  private static final org.apache.thrift.protocol.TField F_FIELD_DESC = new org.apache.thrift.protocol.TField("f", org.apache.thrift.protocol.TType.MAP, (short)1);
  private static final org.apache.thrift.protocol.TField CELLS_FIELD_DESC = new org.apache.thrift.protocol.TField("cells", org.apache.thrift.protocol.TType.LIST, (short)2);

  private static final org.apache.thrift.scheme.SchemeFactory STANDARD_SCHEME_FACTORY = new FCellsStandardSchemeFactory();
  private static final org.apache.thrift.scheme.SchemeFactory TUPLE_SCHEME_FACTORY = new FCellsTupleSchemeFactory();

  public @org.apache.thrift.annotation.Nullable java.util.Map<java.nio.ByteBuffer,FCells> f; // required
  public @org.apache.thrift.annotation.Nullable java.util.List<FCell> cells; // optional

  /** The set of fields this struct contains, along with convenience methods for finding and manipulating them. */
  public enum _Fields implements org.apache.thrift.TFieldIdEnum {
    F((short)1, "f"),
    CELLS((short)2, "cells");

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
        case 1: // F
          return F;
        case 2: // CELLS
          return CELLS;
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
  private static final _Fields optionals[] = {_Fields.CELLS};
  public static final java.util.Map<_Fields, org.apache.thrift.meta_data.FieldMetaData> metaDataMap;
  static {
    java.util.Map<_Fields, org.apache.thrift.meta_data.FieldMetaData> tmpMap = new java.util.EnumMap<_Fields, org.apache.thrift.meta_data.FieldMetaData>(_Fields.class);
    tmpMap.put(_Fields.F, new org.apache.thrift.meta_data.FieldMetaData("f", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.MapMetaData(org.apache.thrift.protocol.TType.MAP, 
            new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.STRING            , true), 
            new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.STRUCT            , "FCells"))));
    tmpMap.put(_Fields.CELLS, new org.apache.thrift.meta_data.FieldMetaData("cells", org.apache.thrift.TFieldRequirementType.OPTIONAL, 
        new org.apache.thrift.meta_data.ListMetaData(org.apache.thrift.protocol.TType.LIST, 
            new org.apache.thrift.meta_data.StructMetaData(org.apache.thrift.protocol.TType.STRUCT, FCell.class))));
    metaDataMap = java.util.Collections.unmodifiableMap(tmpMap);
    org.apache.thrift.meta_data.FieldMetaData.addStructMetaDataMap(FCells.class, metaDataMap);
  }

  public FCells() {
  }

  public FCells(
    java.util.Map<java.nio.ByteBuffer,FCells> f)
  {
    this();
    this.f = f;
  }

  /**
   * Performs a deep copy on <i>other</i>.
   */
  public FCells(FCells other) {
    if (other.isSetF()) {
      java.util.Map<java.nio.ByteBuffer,FCells> __this__f = new java.util.TreeMap<java.nio.ByteBuffer,FCells>();
      for (java.util.Map.Entry<java.nio.ByteBuffer, FCells> other_element : other.f.entrySet()) {

        java.nio.ByteBuffer other_element_key = other_element.getKey();
        FCells other_element_value = other_element.getValue();

        java.nio.ByteBuffer __this__f_copy_key = org.apache.thrift.TBaseHelper.copyBinary(other_element_key);

        FCells __this__f_copy_value = new FCells(other_element_value);

        __this__f.put(__this__f_copy_key, __this__f_copy_value);
      }
      this.f = __this__f;
    }
    if (other.isSetCells()) {
      java.util.List<FCell> __this__cells = new java.util.ArrayList<FCell>(other.cells.size());
      for (FCell other_element : other.cells) {
        __this__cells.add(new FCell(other_element));
      }
      this.cells = __this__cells;
    }
  }

  public FCells deepCopy() {
    return new FCells(this);
  }

  @Override
  public void clear() {
    this.f = null;
    this.cells = null;
  }

  public int getFSize() {
    return (this.f == null) ? 0 : this.f.size();
  }

  public void putToF(java.nio.ByteBuffer key, FCells val) {
    if (this.f == null) {
      this.f = new java.util.TreeMap<java.nio.ByteBuffer,FCells>();
    }
    this.f.put(key, val);
  }

  @org.apache.thrift.annotation.Nullable
  public java.util.Map<java.nio.ByteBuffer,FCells> getF() {
    return this.f;
  }

  public FCells setF(@org.apache.thrift.annotation.Nullable java.util.Map<java.nio.ByteBuffer,FCells> f) {
    this.f = f;
    return this;
  }

  public void unsetF() {
    this.f = null;
  }

  /** Returns true if field f is set (has been assigned a value) and false otherwise */
  public boolean isSetF() {
    return this.f != null;
  }

  public void setFIsSet(boolean value) {
    if (!value) {
      this.f = null;
    }
  }

  public int getCellsSize() {
    return (this.cells == null) ? 0 : this.cells.size();
  }

  @org.apache.thrift.annotation.Nullable
  public java.util.Iterator<FCell> getCellsIterator() {
    return (this.cells == null) ? null : this.cells.iterator();
  }

  public void addToCells(FCell elem) {
    if (this.cells == null) {
      this.cells = new java.util.ArrayList<FCell>();
    }
    this.cells.add(elem);
  }

  @org.apache.thrift.annotation.Nullable
  public java.util.List<FCell> getCells() {
    return this.cells;
  }

  public FCells setCells(@org.apache.thrift.annotation.Nullable java.util.List<FCell> cells) {
    this.cells = cells;
    return this;
  }

  public void unsetCells() {
    this.cells = null;
  }

  /** Returns true if field cells is set (has been assigned a value) and false otherwise */
  public boolean isSetCells() {
    return this.cells != null;
  }

  public void setCellsIsSet(boolean value) {
    if (!value) {
      this.cells = null;
    }
  }

  public void setFieldValue(_Fields field, @org.apache.thrift.annotation.Nullable java.lang.Object value) {
    switch (field) {
    case F:
      if (value == null) {
        unsetF();
      } else {
        setF((java.util.Map<java.nio.ByteBuffer,FCells>)value);
      }
      break;

    case CELLS:
      if (value == null) {
        unsetCells();
      } else {
        setCells((java.util.List<FCell>)value);
      }
      break;

    }
  }

  @org.apache.thrift.annotation.Nullable
  public java.lang.Object getFieldValue(_Fields field) {
    switch (field) {
    case F:
      return getF();

    case CELLS:
      return getCells();

    }
    throw new java.lang.IllegalStateException();
  }

  /** Returns true if field corresponding to fieldID is set (has been assigned a value) and false otherwise */
  public boolean isSet(_Fields field) {
    if (field == null) {
      throw new java.lang.IllegalArgumentException();
    }

    switch (field) {
    case F:
      return isSetF();
    case CELLS:
      return isSetCells();
    }
    throw new java.lang.IllegalStateException();
  }

  @Override
  public boolean equals(java.lang.Object that) {
    if (that == null)
      return false;
    if (that instanceof FCells)
      return this.equals((FCells)that);
    return false;
  }

  public boolean equals(FCells that) {
    if (that == null)
      return false;
    if (this == that)
      return true;

    boolean this_present_f = true && this.isSetF();
    boolean that_present_f = true && that.isSetF();
    if (this_present_f || that_present_f) {
      if (!(this_present_f && that_present_f))
        return false;
      if (!this.f.equals(that.f))
        return false;
    }

    boolean this_present_cells = true && this.isSetCells();
    boolean that_present_cells = true && that.isSetCells();
    if (this_present_cells || that_present_cells) {
      if (!(this_present_cells && that_present_cells))
        return false;
      if (!this.cells.equals(that.cells))
        return false;
    }

    return true;
  }

  @Override
  public int hashCode() {
    int hashCode = 1;

    hashCode = hashCode * 8191 + ((isSetF()) ? 131071 : 524287);
    if (isSetF())
      hashCode = hashCode * 8191 + f.hashCode();

    hashCode = hashCode * 8191 + ((isSetCells()) ? 131071 : 524287);
    if (isSetCells())
      hashCode = hashCode * 8191 + cells.hashCode();

    return hashCode;
  }

  @Override
  public int compareTo(FCells other) {
    if (!getClass().equals(other.getClass())) {
      return getClass().getName().compareTo(other.getClass().getName());
    }

    int lastComparison = 0;

    lastComparison = java.lang.Boolean.valueOf(isSetF()).compareTo(other.isSetF());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetF()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.f, other.f);
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    lastComparison = java.lang.Boolean.valueOf(isSetCells()).compareTo(other.isSetCells());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetCells()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.cells, other.cells);
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
    java.lang.StringBuilder sb = new java.lang.StringBuilder("FCells(");
    boolean first = true;

    sb.append("f:");
    if (this.f == null) {
      sb.append("null");
    } else {
      sb.append(this.f);
    }
    first = false;
    if (isSetCells()) {
      if (!first) sb.append(", ");
      sb.append("cells:");
      if (this.cells == null) {
        sb.append("null");
      } else {
        sb.append(this.cells);
      }
      first = false;
    }
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

  private static class FCellsStandardSchemeFactory implements org.apache.thrift.scheme.SchemeFactory {
    public FCellsStandardScheme getScheme() {
      return new FCellsStandardScheme();
    }
  }

  private static class FCellsStandardScheme extends org.apache.thrift.scheme.StandardScheme<FCells> {

    public void read(org.apache.thrift.protocol.TProtocol iprot, FCells struct) throws org.apache.thrift.TException {
      org.apache.thrift.protocol.TField schemeField;
      iprot.readStructBegin();
      while (true)
      {
        schemeField = iprot.readFieldBegin();
        if (schemeField.type == org.apache.thrift.protocol.TType.STOP) { 
          break;
        }
        switch (schemeField.id) {
          case 1: // F
            if (schemeField.type == org.apache.thrift.protocol.TType.MAP) {
              {
                org.apache.thrift.protocol.TMap _map128 = iprot.readMapBegin();
                struct.f = new java.util.TreeMap<java.nio.ByteBuffer,FCells>();
                @org.apache.thrift.annotation.Nullable java.nio.ByteBuffer _key129;
                @org.apache.thrift.annotation.Nullable FCells _val130;
                for (int _i131 = 0; _i131 < _map128.size; ++_i131)
                {
                  _key129 = iprot.readBinary();
                  _val130 = new FCells();
                  _val130.read(iprot);
                  struct.f.put(_key129, _val130);
                }
                iprot.readMapEnd();
              }
              struct.setFIsSet(true);
            } else { 
              org.apache.thrift.protocol.TProtocolUtil.skip(iprot, schemeField.type);
            }
            break;
          case 2: // CELLS
            if (schemeField.type == org.apache.thrift.protocol.TType.LIST) {
              {
                org.apache.thrift.protocol.TList _list132 = iprot.readListBegin();
                struct.cells = new java.util.ArrayList<FCell>(_list132.size);
                @org.apache.thrift.annotation.Nullable FCell _elem133;
                for (int _i134 = 0; _i134 < _list132.size; ++_i134)
                {
                  _elem133 = new FCell();
                  _elem133.read(iprot);
                  struct.cells.add(_elem133);
                }
                iprot.readListEnd();
              }
              struct.setCellsIsSet(true);
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

    public void write(org.apache.thrift.protocol.TProtocol oprot, FCells struct) throws org.apache.thrift.TException {
      struct.validate();

      oprot.writeStructBegin(STRUCT_DESC);
      if (struct.f != null) {
        oprot.writeFieldBegin(F_FIELD_DESC);
        {
          oprot.writeMapBegin(new org.apache.thrift.protocol.TMap(org.apache.thrift.protocol.TType.STRING, org.apache.thrift.protocol.TType.STRUCT, struct.f.size()));
          for (java.util.Map.Entry<java.nio.ByteBuffer, FCells> _iter135 : struct.f.entrySet())
          {
            oprot.writeBinary(_iter135.getKey());
            _iter135.getValue().write(oprot);
          }
          oprot.writeMapEnd();
        }
        oprot.writeFieldEnd();
      }
      if (struct.cells != null) {
        if (struct.isSetCells()) {
          oprot.writeFieldBegin(CELLS_FIELD_DESC);
          {
            oprot.writeListBegin(new org.apache.thrift.protocol.TList(org.apache.thrift.protocol.TType.STRUCT, struct.cells.size()));
            for (FCell _iter136 : struct.cells)
            {
              _iter136.write(oprot);
            }
            oprot.writeListEnd();
          }
          oprot.writeFieldEnd();
        }
      }
      oprot.writeFieldStop();
      oprot.writeStructEnd();
    }

  }

  private static class FCellsTupleSchemeFactory implements org.apache.thrift.scheme.SchemeFactory {
    public FCellsTupleScheme getScheme() {
      return new FCellsTupleScheme();
    }
  }

  private static class FCellsTupleScheme extends org.apache.thrift.scheme.TupleScheme<FCells> {

    @Override
    public void write(org.apache.thrift.protocol.TProtocol prot, FCells struct) throws org.apache.thrift.TException {
      org.apache.thrift.protocol.TTupleProtocol oprot = (org.apache.thrift.protocol.TTupleProtocol) prot;
      java.util.BitSet optionals = new java.util.BitSet();
      if (struct.isSetF()) {
        optionals.set(0);
      }
      if (struct.isSetCells()) {
        optionals.set(1);
      }
      oprot.writeBitSet(optionals, 2);
      if (struct.isSetF()) {
        {
          oprot.writeI32(struct.f.size());
          for (java.util.Map.Entry<java.nio.ByteBuffer, FCells> _iter137 : struct.f.entrySet())
          {
            oprot.writeBinary(_iter137.getKey());
            _iter137.getValue().write(oprot);
          }
        }
      }
      if (struct.isSetCells()) {
        {
          oprot.writeI32(struct.cells.size());
          for (FCell _iter138 : struct.cells)
          {
            _iter138.write(oprot);
          }
        }
      }
    }

    @Override
    public void read(org.apache.thrift.protocol.TProtocol prot, FCells struct) throws org.apache.thrift.TException {
      org.apache.thrift.protocol.TTupleProtocol iprot = (org.apache.thrift.protocol.TTupleProtocol) prot;
      java.util.BitSet incoming = iprot.readBitSet(2);
      if (incoming.get(0)) {
        {
          org.apache.thrift.protocol.TMap _map139 = new org.apache.thrift.protocol.TMap(org.apache.thrift.protocol.TType.STRING, org.apache.thrift.protocol.TType.STRUCT, iprot.readI32());
          struct.f = new java.util.TreeMap<java.nio.ByteBuffer,FCells>();
          @org.apache.thrift.annotation.Nullable java.nio.ByteBuffer _key140;
          @org.apache.thrift.annotation.Nullable FCells _val141;
          for (int _i142 = 0; _i142 < _map139.size; ++_i142)
          {
            _key140 = iprot.readBinary();
            _val141 = new FCells();
            _val141.read(iprot);
            struct.f.put(_key140, _val141);
          }
        }
        struct.setFIsSet(true);
      }
      if (incoming.get(1)) {
        {
          org.apache.thrift.protocol.TList _list143 = new org.apache.thrift.protocol.TList(org.apache.thrift.protocol.TType.STRUCT, iprot.readI32());
          struct.cells = new java.util.ArrayList<FCell>(_list143.size);
          @org.apache.thrift.annotation.Nullable FCell _elem144;
          for (int _i145 = 0; _i145 < _list143.size; ++_i145)
          {
            _elem144 = new FCell();
            _elem144.read(iprot);
            struct.cells.add(_elem144);
          }
        }
        struct.setCellsIsSet(true);
      }
    }
  }

  private static <S extends org.apache.thrift.scheme.IScheme> S scheme(org.apache.thrift.protocol.TProtocol proto) {
    return (org.apache.thrift.scheme.StandardScheme.class.equals(proto.getScheme()) ? STANDARD_SCHEME_FACTORY : TUPLE_SCHEME_FACTORY).getScheme();
  }
}

