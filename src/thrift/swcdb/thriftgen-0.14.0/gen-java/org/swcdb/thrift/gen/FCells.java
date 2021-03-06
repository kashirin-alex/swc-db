/**
 * Autogenerated by Thrift Compiler (0.14.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
package org.swcdb.thrift.gen;

@SuppressWarnings({"cast", "rawtypes", "serial", "unchecked", "unused"})
/**
 * The Fraction Cells for results on Fraction of scan
 */
public class FCells implements org.apache.thrift.TBase<FCells, FCells._Fields>, java.io.Serializable, Cloneable, Comparable<FCells> {
  private static final org.apache.thrift.protocol.TStruct STRUCT_DESC = new org.apache.thrift.protocol.TStruct("FCells");

  private static final org.apache.thrift.protocol.TField F_FIELD_DESC = new org.apache.thrift.protocol.TField("f", org.apache.thrift.protocol.TType.MAP, (short)1);
  private static final org.apache.thrift.protocol.TField CELLS_FIELD_DESC = new org.apache.thrift.protocol.TField("cells", org.apache.thrift.protocol.TType.LIST, (short)2);
  private static final org.apache.thrift.protocol.TField SERIAL_CELLS_FIELD_DESC = new org.apache.thrift.protocol.TField("serial_cells", org.apache.thrift.protocol.TType.LIST, (short)3);

  private static final org.apache.thrift.scheme.SchemeFactory STANDARD_SCHEME_FACTORY = new FCellsStandardSchemeFactory();
  private static final org.apache.thrift.scheme.SchemeFactory TUPLE_SCHEME_FACTORY = new FCellsTupleSchemeFactory();

  /**
   * The Fraction Container for the Next Fractions Tree,  defined as FCells items in a map-container by current Fraction bytes
   */
  public @org.apache.thrift.annotation.Nullable java.util.Map<java.nio.ByteBuffer,FCells> f; // required
  /**
   * The current Fraction's Cells, defined as FCell items in a list-container
   */
  public @org.apache.thrift.annotation.Nullable java.util.List<FCell> cells; // required
  /**
   * The current Fraction's Serial Cells, defined as FCellSerial items in a list-container
   */
  public @org.apache.thrift.annotation.Nullable java.util.List<FCellSerial> serial_cells; // required

  /** The set of fields this struct contains, along with convenience methods for finding and manipulating them. */
  public enum _Fields implements org.apache.thrift.TFieldIdEnum {
    /**
     * The Fraction Container for the Next Fractions Tree,  defined as FCells items in a map-container by current Fraction bytes
     */
    F((short)1, "f"),
    /**
     * The current Fraction's Cells, defined as FCell items in a list-container
     */
    CELLS((short)2, "cells"),
    /**
     * The current Fraction's Serial Cells, defined as FCellSerial items in a list-container
     */
    SERIAL_CELLS((short)3, "serial_cells");

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
        case 3: // SERIAL_CELLS
          return SERIAL_CELLS;
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
    tmpMap.put(_Fields.F, new org.apache.thrift.meta_data.FieldMetaData("f", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.MapMetaData(org.apache.thrift.protocol.TType.MAP, 
            new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.STRING            , true), 
            new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.STRUCT            , "FCells"))));
    tmpMap.put(_Fields.CELLS, new org.apache.thrift.meta_data.FieldMetaData("cells", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.ListMetaData(org.apache.thrift.protocol.TType.LIST, 
            new org.apache.thrift.meta_data.StructMetaData(org.apache.thrift.protocol.TType.STRUCT, FCell.class))));
    tmpMap.put(_Fields.SERIAL_CELLS, new org.apache.thrift.meta_data.FieldMetaData("serial_cells", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.ListMetaData(org.apache.thrift.protocol.TType.LIST, 
            new org.apache.thrift.meta_data.StructMetaData(org.apache.thrift.protocol.TType.STRUCT, FCellSerial.class))));
    metaDataMap = java.util.Collections.unmodifiableMap(tmpMap);
    org.apache.thrift.meta_data.FieldMetaData.addStructMetaDataMap(FCells.class, metaDataMap);
  }

  public FCells() {
  }

  public FCells(
    java.util.Map<java.nio.ByteBuffer,FCells> f,
    java.util.List<FCell> cells,
    java.util.List<FCellSerial> serial_cells)
  {
    this();
    this.f = f;
    this.cells = cells;
    this.serial_cells = serial_cells;
  }

  /**
   * Performs a deep copy on <i>other</i>.
   */
  public FCells(FCells other) {
    if (other.isSetF()) {
      java.util.Map<java.nio.ByteBuffer,FCells> __this__f = new java.util.HashMap<java.nio.ByteBuffer,FCells>(other.f.size());
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
    if (other.isSetSerial_cells()) {
      java.util.List<FCellSerial> __this__serial_cells = new java.util.ArrayList<FCellSerial>(other.serial_cells.size());
      for (FCellSerial other_element : other.serial_cells) {
        __this__serial_cells.add(new FCellSerial(other_element));
      }
      this.serial_cells = __this__serial_cells;
    }
  }

  public FCells deepCopy() {
    return new FCells(this);
  }

  @Override
  public void clear() {
    this.f = null;
    this.cells = null;
    this.serial_cells = null;
  }

  public int getFSize() {
    return (this.f == null) ? 0 : this.f.size();
  }

  public void putToF(java.nio.ByteBuffer key, FCells val) {
    if (this.f == null) {
      this.f = new java.util.HashMap<java.nio.ByteBuffer,FCells>();
    }
    this.f.put(key, val);
  }

  /**
   * The Fraction Container for the Next Fractions Tree,  defined as FCells items in a map-container by current Fraction bytes
   */
  @org.apache.thrift.annotation.Nullable
  public java.util.Map<java.nio.ByteBuffer,FCells> getF() {
    return this.f;
  }

  /**
   * The Fraction Container for the Next Fractions Tree,  defined as FCells items in a map-container by current Fraction bytes
   */
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

  /**
   * The current Fraction's Cells, defined as FCell items in a list-container
   */
  @org.apache.thrift.annotation.Nullable
  public java.util.List<FCell> getCells() {
    return this.cells;
  }

  /**
   * The current Fraction's Cells, defined as FCell items in a list-container
   */
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

  public int getSerial_cellsSize() {
    return (this.serial_cells == null) ? 0 : this.serial_cells.size();
  }

  @org.apache.thrift.annotation.Nullable
  public java.util.Iterator<FCellSerial> getSerial_cellsIterator() {
    return (this.serial_cells == null) ? null : this.serial_cells.iterator();
  }

  public void addToSerial_cells(FCellSerial elem) {
    if (this.serial_cells == null) {
      this.serial_cells = new java.util.ArrayList<FCellSerial>();
    }
    this.serial_cells.add(elem);
  }

  /**
   * The current Fraction's Serial Cells, defined as FCellSerial items in a list-container
   */
  @org.apache.thrift.annotation.Nullable
  public java.util.List<FCellSerial> getSerial_cells() {
    return this.serial_cells;
  }

  /**
   * The current Fraction's Serial Cells, defined as FCellSerial items in a list-container
   */
  public FCells setSerial_cells(@org.apache.thrift.annotation.Nullable java.util.List<FCellSerial> serial_cells) {
    this.serial_cells = serial_cells;
    return this;
  }

  public void unsetSerial_cells() {
    this.serial_cells = null;
  }

  /** Returns true if field serial_cells is set (has been assigned a value) and false otherwise */
  public boolean isSetSerial_cells() {
    return this.serial_cells != null;
  }

  public void setSerial_cellsIsSet(boolean value) {
    if (!value) {
      this.serial_cells = null;
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

    case SERIAL_CELLS:
      if (value == null) {
        unsetSerial_cells();
      } else {
        setSerial_cells((java.util.List<FCellSerial>)value);
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

    case SERIAL_CELLS:
      return getSerial_cells();

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
    case SERIAL_CELLS:
      return isSetSerial_cells();
    }
    throw new java.lang.IllegalStateException();
  }

  @Override
  public boolean equals(java.lang.Object that) {
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

    boolean this_present_serial_cells = true && this.isSetSerial_cells();
    boolean that_present_serial_cells = true && that.isSetSerial_cells();
    if (this_present_serial_cells || that_present_serial_cells) {
      if (!(this_present_serial_cells && that_present_serial_cells))
        return false;
      if (!this.serial_cells.equals(that.serial_cells))
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

    hashCode = hashCode * 8191 + ((isSetSerial_cells()) ? 131071 : 524287);
    if (isSetSerial_cells())
      hashCode = hashCode * 8191 + serial_cells.hashCode();

    return hashCode;
  }

  @Override
  public int compareTo(FCells other) {
    if (!getClass().equals(other.getClass())) {
      return getClass().getName().compareTo(other.getClass().getName());
    }

    int lastComparison = 0;

    lastComparison = java.lang.Boolean.compare(isSetF(), other.isSetF());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetF()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.f, other.f);
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    lastComparison = java.lang.Boolean.compare(isSetCells(), other.isSetCells());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetCells()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.cells, other.cells);
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    lastComparison = java.lang.Boolean.compare(isSetSerial_cells(), other.isSetSerial_cells());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetSerial_cells()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.serial_cells, other.serial_cells);
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
    if (!first) sb.append(", ");
    sb.append("cells:");
    if (this.cells == null) {
      sb.append("null");
    } else {
      sb.append(this.cells);
    }
    first = false;
    if (!first) sb.append(", ");
    sb.append("serial_cells:");
    if (this.serial_cells == null) {
      sb.append("null");
    } else {
      sb.append(this.serial_cells);
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
                org.apache.thrift.protocol.TMap _map352 = iprot.readMapBegin();
                struct.f = new java.util.HashMap<java.nio.ByteBuffer,FCells>(2*_map352.size);
                @org.apache.thrift.annotation.Nullable java.nio.ByteBuffer _key353;
                @org.apache.thrift.annotation.Nullable FCells _val354;
                for (int _i355 = 0; _i355 < _map352.size; ++_i355)
                {
                  _key353 = iprot.readBinary();
                  _val354 = new FCells();
                  _val354.read(iprot);
                  struct.f.put(_key353, _val354);
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
                org.apache.thrift.protocol.TList _list356 = iprot.readListBegin();
                struct.cells = new java.util.ArrayList<FCell>(_list356.size);
                @org.apache.thrift.annotation.Nullable FCell _elem357;
                for (int _i358 = 0; _i358 < _list356.size; ++_i358)
                {
                  _elem357 = new FCell();
                  _elem357.read(iprot);
                  struct.cells.add(_elem357);
                }
                iprot.readListEnd();
              }
              struct.setCellsIsSet(true);
            } else { 
              org.apache.thrift.protocol.TProtocolUtil.skip(iprot, schemeField.type);
            }
            break;
          case 3: // SERIAL_CELLS
            if (schemeField.type == org.apache.thrift.protocol.TType.LIST) {
              {
                org.apache.thrift.protocol.TList _list359 = iprot.readListBegin();
                struct.serial_cells = new java.util.ArrayList<FCellSerial>(_list359.size);
                @org.apache.thrift.annotation.Nullable FCellSerial _elem360;
                for (int _i361 = 0; _i361 < _list359.size; ++_i361)
                {
                  _elem360 = new FCellSerial();
                  _elem360.read(iprot);
                  struct.serial_cells.add(_elem360);
                }
                iprot.readListEnd();
              }
              struct.setSerial_cellsIsSet(true);
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
          for (java.util.Map.Entry<java.nio.ByteBuffer, FCells> _iter362 : struct.f.entrySet())
          {
            oprot.writeBinary(_iter362.getKey());
            _iter362.getValue().write(oprot);
          }
          oprot.writeMapEnd();
        }
        oprot.writeFieldEnd();
      }
      if (struct.cells != null) {
        oprot.writeFieldBegin(CELLS_FIELD_DESC);
        {
          oprot.writeListBegin(new org.apache.thrift.protocol.TList(org.apache.thrift.protocol.TType.STRUCT, struct.cells.size()));
          for (FCell _iter363 : struct.cells)
          {
            _iter363.write(oprot);
          }
          oprot.writeListEnd();
        }
        oprot.writeFieldEnd();
      }
      if (struct.serial_cells != null) {
        oprot.writeFieldBegin(SERIAL_CELLS_FIELD_DESC);
        {
          oprot.writeListBegin(new org.apache.thrift.protocol.TList(org.apache.thrift.protocol.TType.STRUCT, struct.serial_cells.size()));
          for (FCellSerial _iter364 : struct.serial_cells)
          {
            _iter364.write(oprot);
          }
          oprot.writeListEnd();
        }
        oprot.writeFieldEnd();
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
      if (struct.isSetSerial_cells()) {
        optionals.set(2);
      }
      oprot.writeBitSet(optionals, 3);
      if (struct.isSetF()) {
        {
          oprot.writeI32(struct.f.size());
          for (java.util.Map.Entry<java.nio.ByteBuffer, FCells> _iter365 : struct.f.entrySet())
          {
            oprot.writeBinary(_iter365.getKey());
            _iter365.getValue().write(oprot);
          }
        }
      }
      if (struct.isSetCells()) {
        {
          oprot.writeI32(struct.cells.size());
          for (FCell _iter366 : struct.cells)
          {
            _iter366.write(oprot);
          }
        }
      }
      if (struct.isSetSerial_cells()) {
        {
          oprot.writeI32(struct.serial_cells.size());
          for (FCellSerial _iter367 : struct.serial_cells)
          {
            _iter367.write(oprot);
          }
        }
      }
    }

    @Override
    public void read(org.apache.thrift.protocol.TProtocol prot, FCells struct) throws org.apache.thrift.TException {
      org.apache.thrift.protocol.TTupleProtocol iprot = (org.apache.thrift.protocol.TTupleProtocol) prot;
      java.util.BitSet incoming = iprot.readBitSet(3);
      if (incoming.get(0)) {
        {
          org.apache.thrift.protocol.TMap _map368 = iprot.readMapBegin(org.apache.thrift.protocol.TType.STRING, org.apache.thrift.protocol.TType.STRUCT); 
          struct.f = new java.util.HashMap<java.nio.ByteBuffer,FCells>(2*_map368.size);
          @org.apache.thrift.annotation.Nullable java.nio.ByteBuffer _key369;
          @org.apache.thrift.annotation.Nullable FCells _val370;
          for (int _i371 = 0; _i371 < _map368.size; ++_i371)
          {
            _key369 = iprot.readBinary();
            _val370 = new FCells();
            _val370.read(iprot);
            struct.f.put(_key369, _val370);
          }
        }
        struct.setFIsSet(true);
      }
      if (incoming.get(1)) {
        {
          org.apache.thrift.protocol.TList _list372 = iprot.readListBegin(org.apache.thrift.protocol.TType.STRUCT);
          struct.cells = new java.util.ArrayList<FCell>(_list372.size);
          @org.apache.thrift.annotation.Nullable FCell _elem373;
          for (int _i374 = 0; _i374 < _list372.size; ++_i374)
          {
            _elem373 = new FCell();
            _elem373.read(iprot);
            struct.cells.add(_elem373);
          }
        }
        struct.setCellsIsSet(true);
      }
      if (incoming.get(2)) {
        {
          org.apache.thrift.protocol.TList _list375 = iprot.readListBegin(org.apache.thrift.protocol.TType.STRUCT);
          struct.serial_cells = new java.util.ArrayList<FCellSerial>(_list375.size);
          @org.apache.thrift.annotation.Nullable FCellSerial _elem376;
          for (int _i377 = 0; _i377 < _list375.size; ++_i377)
          {
            _elem376 = new FCellSerial();
            _elem376.read(iprot);
            struct.serial_cells.add(_elem376);
          }
        }
        struct.setSerial_cellsIsSet(true);
      }
    }
  }

  private static <S extends org.apache.thrift.scheme.IScheme> S scheme(org.apache.thrift.protocol.TProtocol proto) {
    return (org.apache.thrift.scheme.StandardScheme.class.equals(proto.getScheme()) ? STANDARD_SCHEME_FACTORY : TUPLE_SCHEME_FACTORY).getScheme();
  }
}

