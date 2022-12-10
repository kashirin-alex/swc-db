/**
 * Autogenerated by Thrift Compiler (0.17.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
package org.swcdb.thrift.gen;

/**
 * The Result of 'exec_sql'
 */
@SuppressWarnings({"cast", "rawtypes", "serial", "unchecked", "unused"})
public class Result implements org.apache.thrift.TBase<Result, Result._Fields>, java.io.Serializable, Cloneable, Comparable<Result> {
  private static final org.apache.thrift.protocol.TStruct STRUCT_DESC = new org.apache.thrift.protocol.TStruct("Result");

  private static final org.apache.thrift.protocol.TField SCHEMAS_FIELD_DESC = new org.apache.thrift.protocol.TField("schemas", org.apache.thrift.protocol.TType.LIST, (short)1);
  private static final org.apache.thrift.protocol.TField CELLS_FIELD_DESC = new org.apache.thrift.protocol.TField("cells", org.apache.thrift.protocol.TType.STRUCT, (short)2);
  private static final org.apache.thrift.protocol.TField COMPACT_FIELD_DESC = new org.apache.thrift.protocol.TField("compact", org.apache.thrift.protocol.TType.LIST, (short)3);

  private static final org.apache.thrift.scheme.SchemeFactory STANDARD_SCHEME_FACTORY = new ResultStandardSchemeFactory();
  private static final org.apache.thrift.scheme.SchemeFactory TUPLE_SCHEME_FACTORY = new ResultTupleSchemeFactory();

  /**
   * Set with result for 'list columns' query
   */
  public @org.apache.thrift.annotation.Nullable java.util.List<Schema> schemas; // required
  /**
   * Set with result for 'select' query
   */
  public @org.apache.thrift.annotation.Nullable Cells cells; // required
  /**
   * Set with result for 'compact columns' query
   */
  public @org.apache.thrift.annotation.Nullable java.util.List<CompactResult> compact; // required

  /** The set of fields this struct contains, along with convenience methods for finding and manipulating them. */
  public enum _Fields implements org.apache.thrift.TFieldIdEnum {
    /**
     * Set with result for 'list columns' query
     */
    SCHEMAS((short)1, "schemas"),
    /**
     * Set with result for 'select' query
     */
    CELLS((short)2, "cells"),
    /**
     * Set with result for 'compact columns' query
     */
    COMPACT((short)3, "compact");

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
        case 1: // SCHEMAS
          return SCHEMAS;
        case 2: // CELLS
          return CELLS;
        case 3: // COMPACT
          return COMPACT;
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

    @Override
    public short getThriftFieldId() {
      return _thriftId;
    }

    @Override
    public java.lang.String getFieldName() {
      return _fieldName;
    }
  }

  // isset id assignments
  public static final java.util.Map<_Fields, org.apache.thrift.meta_data.FieldMetaData> metaDataMap;
  static {
    java.util.Map<_Fields, org.apache.thrift.meta_data.FieldMetaData> tmpMap = new java.util.EnumMap<_Fields, org.apache.thrift.meta_data.FieldMetaData>(_Fields.class);
    tmpMap.put(_Fields.SCHEMAS, new org.apache.thrift.meta_data.FieldMetaData("schemas", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.LIST        , "Schemas")));
    tmpMap.put(_Fields.CELLS, new org.apache.thrift.meta_data.FieldMetaData("cells", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.StructMetaData(org.apache.thrift.protocol.TType.STRUCT, Cells.class)));
    tmpMap.put(_Fields.COMPACT, new org.apache.thrift.meta_data.FieldMetaData("compact", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.LIST        , "CompactResults")));
    metaDataMap = java.util.Collections.unmodifiableMap(tmpMap);
    org.apache.thrift.meta_data.FieldMetaData.addStructMetaDataMap(Result.class, metaDataMap);
  }

  public Result() {
  }

  public Result(
    java.util.List<Schema> schemas,
    Cells cells,
    java.util.List<CompactResult> compact)
  {
    this();
    this.schemas = schemas;
    this.cells = cells;
    this.compact = compact;
  }

  /**
   * Performs a deep copy on <i>other</i>.
   */
  public Result(Result other) {
    if (other.isSetSchemas()) {
      java.util.List<Schema> __this__schemas = new java.util.ArrayList<Schema>(other.schemas.size());
      for (Schema other_element : other.schemas) {
        __this__schemas.add(new Schema(other_element));
      }
      this.schemas = __this__schemas;
    }
    if (other.isSetCells()) {
      this.cells = new Cells(other.cells);
    }
    if (other.isSetCompact()) {
      java.util.List<CompactResult> __this__compact = new java.util.ArrayList<CompactResult>(other.compact.size());
      for (CompactResult other_element : other.compact) {
        __this__compact.add(new CompactResult(other_element));
      }
      this.compact = __this__compact;
    }
  }

  @Override
  public Result deepCopy() {
    return new Result(this);
  }

  @Override
  public void clear() {
    this.schemas = null;
    this.cells = null;
    this.compact = null;
  }

  public int getSchemasSize() {
    return (this.schemas == null) ? 0 : this.schemas.size();
  }

  @org.apache.thrift.annotation.Nullable
  public java.util.Iterator<Schema> getSchemasIterator() {
    return (this.schemas == null) ? null : this.schemas.iterator();
  }

  public void addToSchemas(Schema elem) {
    if (this.schemas == null) {
      this.schemas = new java.util.ArrayList<Schema>();
    }
    this.schemas.add(elem);
  }

  /**
   * Set with result for 'list columns' query
   */
  @org.apache.thrift.annotation.Nullable
  public java.util.List<Schema> getSchemas() {
    return this.schemas;
  }

  /**
   * Set with result for 'list columns' query
   */
  public Result setSchemas(@org.apache.thrift.annotation.Nullable java.util.List<Schema> schemas) {
    this.schemas = schemas;
    return this;
  }

  public void unsetSchemas() {
    this.schemas = null;
  }

  /** Returns true if field schemas is set (has been assigned a value) and false otherwise */
  public boolean isSetSchemas() {
    return this.schemas != null;
  }

  public void setSchemasIsSet(boolean value) {
    if (!value) {
      this.schemas = null;
    }
  }

  /**
   * Set with result for 'select' query
   */
  @org.apache.thrift.annotation.Nullable
  public Cells getCells() {
    return this.cells;
  }

  /**
   * Set with result for 'select' query
   */
  public Result setCells(@org.apache.thrift.annotation.Nullable Cells cells) {
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

  public int getCompactSize() {
    return (this.compact == null) ? 0 : this.compact.size();
  }

  @org.apache.thrift.annotation.Nullable
  public java.util.Iterator<CompactResult> getCompactIterator() {
    return (this.compact == null) ? null : this.compact.iterator();
  }

  public void addToCompact(CompactResult elem) {
    if (this.compact == null) {
      this.compact = new java.util.ArrayList<CompactResult>();
    }
    this.compact.add(elem);
  }

  /**
   * Set with result for 'compact columns' query
   */
  @org.apache.thrift.annotation.Nullable
  public java.util.List<CompactResult> getCompact() {
    return this.compact;
  }

  /**
   * Set with result for 'compact columns' query
   */
  public Result setCompact(@org.apache.thrift.annotation.Nullable java.util.List<CompactResult> compact) {
    this.compact = compact;
    return this;
  }

  public void unsetCompact() {
    this.compact = null;
  }

  /** Returns true if field compact is set (has been assigned a value) and false otherwise */
  public boolean isSetCompact() {
    return this.compact != null;
  }

  public void setCompactIsSet(boolean value) {
    if (!value) {
      this.compact = null;
    }
  }

  @Override
  public void setFieldValue(_Fields field, @org.apache.thrift.annotation.Nullable java.lang.Object value) {
    switch (field) {
    case SCHEMAS:
      if (value == null) {
        unsetSchemas();
      } else {
        setSchemas((java.util.List<Schema>)value);
      }
      break;

    case CELLS:
      if (value == null) {
        unsetCells();
      } else {
        setCells((Cells)value);
      }
      break;

    case COMPACT:
      if (value == null) {
        unsetCompact();
      } else {
        setCompact((java.util.List<CompactResult>)value);
      }
      break;

    }
  }

  @org.apache.thrift.annotation.Nullable
  @Override
  public java.lang.Object getFieldValue(_Fields field) {
    switch (field) {
    case SCHEMAS:
      return getSchemas();

    case CELLS:
      return getCells();

    case COMPACT:
      return getCompact();

    }
    throw new java.lang.IllegalStateException();
  }

  /** Returns true if field corresponding to fieldID is set (has been assigned a value) and false otherwise */
  @Override
  public boolean isSet(_Fields field) {
    if (field == null) {
      throw new java.lang.IllegalArgumentException();
    }

    switch (field) {
    case SCHEMAS:
      return isSetSchemas();
    case CELLS:
      return isSetCells();
    case COMPACT:
      return isSetCompact();
    }
    throw new java.lang.IllegalStateException();
  }

  @Override
  public boolean equals(java.lang.Object that) {
    if (that instanceof Result)
      return this.equals((Result)that);
    return false;
  }

  public boolean equals(Result that) {
    if (that == null)
      return false;
    if (this == that)
      return true;

    boolean this_present_schemas = true && this.isSetSchemas();
    boolean that_present_schemas = true && that.isSetSchemas();
    if (this_present_schemas || that_present_schemas) {
      if (!(this_present_schemas && that_present_schemas))
        return false;
      if (!this.schemas.equals(that.schemas))
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

    boolean this_present_compact = true && this.isSetCompact();
    boolean that_present_compact = true && that.isSetCompact();
    if (this_present_compact || that_present_compact) {
      if (!(this_present_compact && that_present_compact))
        return false;
      if (!this.compact.equals(that.compact))
        return false;
    }

    return true;
  }

  @Override
  public int hashCode() {
    int hashCode = 1;

    hashCode = hashCode * 8191 + ((isSetSchemas()) ? 131071 : 524287);
    if (isSetSchemas())
      hashCode = hashCode * 8191 + schemas.hashCode();

    hashCode = hashCode * 8191 + ((isSetCells()) ? 131071 : 524287);
    if (isSetCells())
      hashCode = hashCode * 8191 + cells.hashCode();

    hashCode = hashCode * 8191 + ((isSetCompact()) ? 131071 : 524287);
    if (isSetCompact())
      hashCode = hashCode * 8191 + compact.hashCode();

    return hashCode;
  }

  @Override
  public int compareTo(Result other) {
    if (!getClass().equals(other.getClass())) {
      return getClass().getName().compareTo(other.getClass().getName());
    }

    int lastComparison = 0;

    lastComparison = java.lang.Boolean.compare(isSetSchemas(), other.isSetSchemas());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetSchemas()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.schemas, other.schemas);
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
    lastComparison = java.lang.Boolean.compare(isSetCompact(), other.isSetCompact());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetCompact()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.compact, other.compact);
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    return 0;
  }

  @org.apache.thrift.annotation.Nullable
  @Override
  public _Fields fieldForId(int fieldId) {
    return _Fields.findByThriftId(fieldId);
  }

  @Override
  public void read(org.apache.thrift.protocol.TProtocol iprot) throws org.apache.thrift.TException {
    scheme(iprot).read(iprot, this);
  }

  @Override
  public void write(org.apache.thrift.protocol.TProtocol oprot) throws org.apache.thrift.TException {
    scheme(oprot).write(oprot, this);
  }

  @Override
  public java.lang.String toString() {
    java.lang.StringBuilder sb = new java.lang.StringBuilder("Result(");
    boolean first = true;

    sb.append("schemas:");
    if (this.schemas == null) {
      sb.append("null");
    } else {
      sb.append(this.schemas);
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
    sb.append("compact:");
    if (this.compact == null) {
      sb.append("null");
    } else {
      sb.append(this.compact);
    }
    first = false;
    sb.append(")");
    return sb.toString();
  }

  public void validate() throws org.apache.thrift.TException {
    // check for required fields
    // check for sub-struct validity
    if (cells != null) {
      cells.validate();
    }
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

  private static class ResultStandardSchemeFactory implements org.apache.thrift.scheme.SchemeFactory {
    @Override
    public ResultStandardScheme getScheme() {
      return new ResultStandardScheme();
    }
  }

  private static class ResultStandardScheme extends org.apache.thrift.scheme.StandardScheme<Result> {

    @Override
    public void read(org.apache.thrift.protocol.TProtocol iprot, Result struct) throws org.apache.thrift.TException {
      org.apache.thrift.protocol.TField schemeField;
      iprot.readStructBegin();
      while (true)
      {
        schemeField = iprot.readFieldBegin();
        if (schemeField.type == org.apache.thrift.protocol.TType.STOP) { 
          break;
        }
        switch (schemeField.id) {
          case 1: // SCHEMAS
            if (schemeField.type == org.apache.thrift.protocol.TType.LIST) {
              {
                org.apache.thrift.protocol.TList _list556 = iprot.readListBegin();
                struct.schemas = new java.util.ArrayList<Schema>(_list556.size);
                @org.apache.thrift.annotation.Nullable Schema _elem557;
                for (int _i558 = 0; _i558 < _list556.size; ++_i558)
                {
                  _elem557 = new Schema();
                  _elem557.read(iprot);
                  struct.schemas.add(_elem557);
                }
                iprot.readListEnd();
              }
              struct.setSchemasIsSet(true);
            } else { 
              org.apache.thrift.protocol.TProtocolUtil.skip(iprot, schemeField.type);
            }
            break;
          case 2: // CELLS
            if (schemeField.type == org.apache.thrift.protocol.TType.STRUCT) {
              struct.cells = new Cells();
              struct.cells.read(iprot);
              struct.setCellsIsSet(true);
            } else { 
              org.apache.thrift.protocol.TProtocolUtil.skip(iprot, schemeField.type);
            }
            break;
          case 3: // COMPACT
            if (schemeField.type == org.apache.thrift.protocol.TType.LIST) {
              {
                org.apache.thrift.protocol.TList _list559 = iprot.readListBegin();
                struct.compact = new java.util.ArrayList<CompactResult>(_list559.size);
                @org.apache.thrift.annotation.Nullable CompactResult _elem560;
                for (int _i561 = 0; _i561 < _list559.size; ++_i561)
                {
                  _elem560 = new CompactResult();
                  _elem560.read(iprot);
                  struct.compact.add(_elem560);
                }
                iprot.readListEnd();
              }
              struct.setCompactIsSet(true);
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

    @Override
    public void write(org.apache.thrift.protocol.TProtocol oprot, Result struct) throws org.apache.thrift.TException {
      struct.validate();

      oprot.writeStructBegin(STRUCT_DESC);
      if (struct.schemas != null) {
        oprot.writeFieldBegin(SCHEMAS_FIELD_DESC);
        {
          oprot.writeListBegin(new org.apache.thrift.protocol.TList(org.apache.thrift.protocol.TType.STRUCT, struct.schemas.size()));
          for (Schema _iter562 : struct.schemas)
          {
            _iter562.write(oprot);
          }
          oprot.writeListEnd();
        }
        oprot.writeFieldEnd();
      }
      if (struct.cells != null) {
        oprot.writeFieldBegin(CELLS_FIELD_DESC);
        struct.cells.write(oprot);
        oprot.writeFieldEnd();
      }
      if (struct.compact != null) {
        oprot.writeFieldBegin(COMPACT_FIELD_DESC);
        {
          oprot.writeListBegin(new org.apache.thrift.protocol.TList(org.apache.thrift.protocol.TType.STRUCT, struct.compact.size()));
          for (CompactResult _iter563 : struct.compact)
          {
            _iter563.write(oprot);
          }
          oprot.writeListEnd();
        }
        oprot.writeFieldEnd();
      }
      oprot.writeFieldStop();
      oprot.writeStructEnd();
    }

  }

  private static class ResultTupleSchemeFactory implements org.apache.thrift.scheme.SchemeFactory {
    @Override
    public ResultTupleScheme getScheme() {
      return new ResultTupleScheme();
    }
  }

  private static class ResultTupleScheme extends org.apache.thrift.scheme.TupleScheme<Result> {

    @Override
    public void write(org.apache.thrift.protocol.TProtocol prot, Result struct) throws org.apache.thrift.TException {
      org.apache.thrift.protocol.TTupleProtocol oprot = (org.apache.thrift.protocol.TTupleProtocol) prot;
      java.util.BitSet optionals = new java.util.BitSet();
      if (struct.isSetSchemas()) {
        optionals.set(0);
      }
      if (struct.isSetCells()) {
        optionals.set(1);
      }
      if (struct.isSetCompact()) {
        optionals.set(2);
      }
      oprot.writeBitSet(optionals, 3);
      if (struct.isSetSchemas()) {
        {
          oprot.writeI32(struct.schemas.size());
          for (Schema _iter564 : struct.schemas)
          {
            _iter564.write(oprot);
          }
        }
      }
      if (struct.isSetCells()) {
        struct.cells.write(oprot);
      }
      if (struct.isSetCompact()) {
        {
          oprot.writeI32(struct.compact.size());
          for (CompactResult _iter565 : struct.compact)
          {
            _iter565.write(oprot);
          }
        }
      }
    }

    @Override
    public void read(org.apache.thrift.protocol.TProtocol prot, Result struct) throws org.apache.thrift.TException {
      org.apache.thrift.protocol.TTupleProtocol iprot = (org.apache.thrift.protocol.TTupleProtocol) prot;
      java.util.BitSet incoming = iprot.readBitSet(3);
      if (incoming.get(0)) {
        {
          org.apache.thrift.protocol.TList _list566 = iprot.readListBegin(org.apache.thrift.protocol.TType.STRUCT);
          struct.schemas = new java.util.ArrayList<Schema>(_list566.size);
          @org.apache.thrift.annotation.Nullable Schema _elem567;
          for (int _i568 = 0; _i568 < _list566.size; ++_i568)
          {
            _elem567 = new Schema();
            _elem567.read(iprot);
            struct.schemas.add(_elem567);
          }
        }
        struct.setSchemasIsSet(true);
      }
      if (incoming.get(1)) {
        struct.cells = new Cells();
        struct.cells.read(iprot);
        struct.setCellsIsSet(true);
      }
      if (incoming.get(2)) {
        {
          org.apache.thrift.protocol.TList _list569 = iprot.readListBegin(org.apache.thrift.protocol.TType.STRUCT);
          struct.compact = new java.util.ArrayList<CompactResult>(_list569.size);
          @org.apache.thrift.annotation.Nullable CompactResult _elem570;
          for (int _i571 = 0; _i571 < _list569.size; ++_i571)
          {
            _elem570 = new CompactResult();
            _elem570.read(iprot);
            struct.compact.add(_elem570);
          }
        }
        struct.setCompactIsSet(true);
      }
    }
  }

  private static <S extends org.apache.thrift.scheme.IScheme> S scheme(org.apache.thrift.protocol.TProtocol proto) {
    return (org.apache.thrift.scheme.StandardScheme.class.equals(proto.getScheme()) ? STANDARD_SCHEME_FACTORY : TUPLE_SCHEME_FACTORY).getScheme();
  }
}

