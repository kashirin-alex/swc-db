/**
 * <auto-generated>
 * Autogenerated by Thrift Compiler (0.16.0)
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 * </auto-generated>
 */
using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.Extensions.Logging;
using Thrift;
using Thrift.Collections;

using Thrift.Protocol;
using Thrift.Protocol.Entities;
using Thrift.Protocol.Utilities;
using Thrift.Transport;
using Thrift.Transport.Client;
using Thrift.Transport.Server;
using Thrift.Processor;


#nullable disable                // suppress C# 8.0 nullable contexts (we still support earlier versions)
#pragma warning disable IDE0079  // remove unnecessary pragmas
#pragma warning disable IDE1006  // parts of the code use IDL spelling
#pragma warning disable IDE0083  // pattern matching "that is not SomeType" requires net5.0 but we still support earlier versions


/// <summary>
/// The Scan Specifications, the Columns-Intervals(SpecColumn/s) with global-scope Flags
/// </summary>
public partial class SpecScan : TBase
{
  private List<SpecColumn> _columns;
  private List<SpecColumnSerial> _columns_serial;
  private SpecFlags _flags;

  /// <summary>
  /// The Column Intervals(SpecColumn) in a list-container
  /// </summary>
  public List<SpecColumn> Columns
  {
    get
    {
      return _columns;
    }
    set
    {
      __isset.columns = true;
      this._columns = value;
    }
  }

  /// <summary>
  /// The Serial Column Intervals(SpecColumnSerial) in a list-container
  /// </summary>
  public List<SpecColumnSerial> Columns_serial
  {
    get
    {
      return _columns_serial;
    }
    set
    {
      __isset.columns_serial = true;
      this._columns_serial = value;
    }
  }

  /// <summary>
  /// The Global Flags Specification
  /// </summary>
  public SpecFlags Flags
  {
    get
    {
      return _flags;
    }
    set
    {
      __isset.flags = true;
      this._flags = value;
    }
  }


  public Isset __isset;
  public struct Isset
  {
    public bool columns;
    public bool columns_serial;
    public bool flags;
  }

  public SpecScan()
  {
  }

  public SpecScan DeepCopy()
  {
    var tmp230 = new SpecScan();
    if((Columns != null) && __isset.columns)
    {
      tmp230.Columns = this.Columns.DeepCopy();
    }
    tmp230.__isset.columns = this.__isset.columns;
    if((Columns_serial != null) && __isset.columns_serial)
    {
      tmp230.Columns_serial = this.Columns_serial.DeepCopy();
    }
    tmp230.__isset.columns_serial = this.__isset.columns_serial;
    if((Flags != null) && __isset.flags)
    {
      tmp230.Flags = (SpecFlags)this.Flags.DeepCopy();
    }
    tmp230.__isset.flags = this.__isset.flags;
    return tmp230;
  }

  public async global::System.Threading.Tasks.Task ReadAsync(TProtocol iprot, CancellationToken cancellationToken)
  {
    iprot.IncrementRecursionDepth();
    try
    {
      TField field;
      await iprot.ReadStructBeginAsync(cancellationToken);
      while (true)
      {
        field = await iprot.ReadFieldBeginAsync(cancellationToken);
        if (field.Type == TType.Stop)
        {
          break;
        }

        switch (field.ID)
        {
          case 1:
            if (field.Type == TType.List)
            {
              {
                TList _list231 = await iprot.ReadListBeginAsync(cancellationToken);
                Columns = new List<SpecColumn>(_list231.Count);
                for(int _i232 = 0; _i232 < _list231.Count; ++_i232)
                {
                  SpecColumn _elem233;
                  _elem233 = new SpecColumn();
                  await _elem233.ReadAsync(iprot, cancellationToken);
                  Columns.Add(_elem233);
                }
                await iprot.ReadListEndAsync(cancellationToken);
              }
            }
            else
            {
              await TProtocolUtil.SkipAsync(iprot, field.Type, cancellationToken);
            }
            break;
          case 2:
            if (field.Type == TType.List)
            {
              {
                TList _list234 = await iprot.ReadListBeginAsync(cancellationToken);
                Columns_serial = new List<SpecColumnSerial>(_list234.Count);
                for(int _i235 = 0; _i235 < _list234.Count; ++_i235)
                {
                  SpecColumnSerial _elem236;
                  _elem236 = new SpecColumnSerial();
                  await _elem236.ReadAsync(iprot, cancellationToken);
                  Columns_serial.Add(_elem236);
                }
                await iprot.ReadListEndAsync(cancellationToken);
              }
            }
            else
            {
              await TProtocolUtil.SkipAsync(iprot, field.Type, cancellationToken);
            }
            break;
          case 3:
            if (field.Type == TType.Struct)
            {
              Flags = new SpecFlags();
              await Flags.ReadAsync(iprot, cancellationToken);
            }
            else
            {
              await TProtocolUtil.SkipAsync(iprot, field.Type, cancellationToken);
            }
            break;
          default: 
            await TProtocolUtil.SkipAsync(iprot, field.Type, cancellationToken);
            break;
        }

        await iprot.ReadFieldEndAsync(cancellationToken);
      }

      await iprot.ReadStructEndAsync(cancellationToken);
    }
    finally
    {
      iprot.DecrementRecursionDepth();
    }
  }

  public async global::System.Threading.Tasks.Task WriteAsync(TProtocol oprot, CancellationToken cancellationToken)
  {
    oprot.IncrementRecursionDepth();
    try
    {
      var tmp237 = new TStruct("SpecScan");
      await oprot.WriteStructBeginAsync(tmp237, cancellationToken);
      var tmp238 = new TField();
      if((Columns != null) && __isset.columns)
      {
        tmp238.Name = "columns";
        tmp238.Type = TType.List;
        tmp238.ID = 1;
        await oprot.WriteFieldBeginAsync(tmp238, cancellationToken);
        {
          await oprot.WriteListBeginAsync(new TList(TType.Struct, Columns.Count), cancellationToken);
          foreach (SpecColumn _iter239 in Columns)
          {
            await _iter239.WriteAsync(oprot, cancellationToken);
          }
          await oprot.WriteListEndAsync(cancellationToken);
        }
        await oprot.WriteFieldEndAsync(cancellationToken);
      }
      if((Columns_serial != null) && __isset.columns_serial)
      {
        tmp238.Name = "columns_serial";
        tmp238.Type = TType.List;
        tmp238.ID = 2;
        await oprot.WriteFieldBeginAsync(tmp238, cancellationToken);
        {
          await oprot.WriteListBeginAsync(new TList(TType.Struct, Columns_serial.Count), cancellationToken);
          foreach (SpecColumnSerial _iter240 in Columns_serial)
          {
            await _iter240.WriteAsync(oprot, cancellationToken);
          }
          await oprot.WriteListEndAsync(cancellationToken);
        }
        await oprot.WriteFieldEndAsync(cancellationToken);
      }
      if((Flags != null) && __isset.flags)
      {
        tmp238.Name = "flags";
        tmp238.Type = TType.Struct;
        tmp238.ID = 3;
        await oprot.WriteFieldBeginAsync(tmp238, cancellationToken);
        await Flags.WriteAsync(oprot, cancellationToken);
        await oprot.WriteFieldEndAsync(cancellationToken);
      }
      await oprot.WriteFieldStopAsync(cancellationToken);
      await oprot.WriteStructEndAsync(cancellationToken);
    }
    finally
    {
      oprot.DecrementRecursionDepth();
    }
  }

  public override bool Equals(object that)
  {
    if (!(that is SpecScan other)) return false;
    if (ReferenceEquals(this, other)) return true;
    return ((__isset.columns == other.__isset.columns) && ((!__isset.columns) || (TCollections.Equals(Columns, other.Columns))))
      && ((__isset.columns_serial == other.__isset.columns_serial) && ((!__isset.columns_serial) || (TCollections.Equals(Columns_serial, other.Columns_serial))))
      && ((__isset.flags == other.__isset.flags) && ((!__isset.flags) || (global::System.Object.Equals(Flags, other.Flags))));
  }

  public override int GetHashCode() {
    int hashcode = 157;
    unchecked {
      if((Columns != null) && __isset.columns)
      {
        hashcode = (hashcode * 397) + TCollections.GetHashCode(Columns);
      }
      if((Columns_serial != null) && __isset.columns_serial)
      {
        hashcode = (hashcode * 397) + TCollections.GetHashCode(Columns_serial);
      }
      if((Flags != null) && __isset.flags)
      {
        hashcode = (hashcode * 397) + Flags.GetHashCode();
      }
    }
    return hashcode;
  }

  public override string ToString()
  {
    var tmp241 = new StringBuilder("SpecScan(");
    int tmp242 = 0;
    if((Columns != null) && __isset.columns)
    {
      if(0 < tmp242++) { tmp241.Append(", "); }
      tmp241.Append("Columns: ");
      Columns.ToString(tmp241);
    }
    if((Columns_serial != null) && __isset.columns_serial)
    {
      if(0 < tmp242++) { tmp241.Append(", "); }
      tmp241.Append("Columns_serial: ");
      Columns_serial.ToString(tmp241);
    }
    if((Flags != null) && __isset.flags)
    {
      if(0 < tmp242++) { tmp241.Append(", "); }
      tmp241.Append("Flags: ");
      Flags.ToString(tmp241);
    }
    tmp241.Append(')');
    return tmp241.ToString();
  }
}

