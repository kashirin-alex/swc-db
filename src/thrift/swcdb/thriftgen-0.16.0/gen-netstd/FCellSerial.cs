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
/// The Fraction Serial Cell for results on Fraction of scan
/// </summary>
public partial class FCellSerial : TBase
{
  private string _c;
  private long _ts;
  private List<CellValueSerial> _v;

  /// <summary>
  /// The Column Name
  /// </summary>
  public string C
  {
    get
    {
      return _c;
    }
    set
    {
      __isset.c = true;
      this._c = value;
    }
  }

  /// <summary>
  /// The Cell Timestamp
  /// </summary>
  public long Ts
  {
    get
    {
      return _ts;
    }
    set
    {
      __isset.ts = true;
      this._ts = value;
    }
  }

  /// <summary>
  /// The Cell Serial Value
  /// </summary>
  public List<CellValueSerial> V
  {
    get
    {
      return _v;
    }
    set
    {
      __isset.v = true;
      this._v = value;
    }
  }


  public Isset __isset;
  public struct Isset
  {
    public bool c;
    public bool ts;
    public bool v;
  }

  public FCellSerial()
  {
  }

  public FCellSerial DeepCopy()
  {
    var tmp430 = new FCellSerial();
    if((C != null) && __isset.c)
    {
      tmp430.C = this.C;
    }
    tmp430.__isset.c = this.__isset.c;
    if(__isset.ts)
    {
      tmp430.Ts = this.Ts;
    }
    tmp430.__isset.ts = this.__isset.ts;
    if((V != null) && __isset.v)
    {
      tmp430.V = this.V.DeepCopy();
    }
    tmp430.__isset.v = this.__isset.v;
    return tmp430;
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
            if (field.Type == TType.String)
            {
              C = await iprot.ReadStringAsync(cancellationToken);
            }
            else
            {
              await TProtocolUtil.SkipAsync(iprot, field.Type, cancellationToken);
            }
            break;
          case 2:
            if (field.Type == TType.I64)
            {
              Ts = await iprot.ReadI64Async(cancellationToken);
            }
            else
            {
              await TProtocolUtil.SkipAsync(iprot, field.Type, cancellationToken);
            }
            break;
          case 3:
            if (field.Type == TType.List)
            {
              {
                TList _list431 = await iprot.ReadListBeginAsync(cancellationToken);
                V = new List<CellValueSerial>(_list431.Count);
                for(int _i432 = 0; _i432 < _list431.Count; ++_i432)
                {
                  CellValueSerial _elem433;
                  _elem433 = new CellValueSerial();
                  await _elem433.ReadAsync(iprot, cancellationToken);
                  V.Add(_elem433);
                }
                await iprot.ReadListEndAsync(cancellationToken);
              }
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
      var tmp434 = new TStruct("FCellSerial");
      await oprot.WriteStructBeginAsync(tmp434, cancellationToken);
      var tmp435 = new TField();
      if((C != null) && __isset.c)
      {
        tmp435.Name = "c";
        tmp435.Type = TType.String;
        tmp435.ID = 1;
        await oprot.WriteFieldBeginAsync(tmp435, cancellationToken);
        await oprot.WriteStringAsync(C, cancellationToken);
        await oprot.WriteFieldEndAsync(cancellationToken);
      }
      if(__isset.ts)
      {
        tmp435.Name = "ts";
        tmp435.Type = TType.I64;
        tmp435.ID = 2;
        await oprot.WriteFieldBeginAsync(tmp435, cancellationToken);
        await oprot.WriteI64Async(Ts, cancellationToken);
        await oprot.WriteFieldEndAsync(cancellationToken);
      }
      if((V != null) && __isset.v)
      {
        tmp435.Name = "v";
        tmp435.Type = TType.List;
        tmp435.ID = 3;
        await oprot.WriteFieldBeginAsync(tmp435, cancellationToken);
        {
          await oprot.WriteListBeginAsync(new TList(TType.Struct, V.Count), cancellationToken);
          foreach (CellValueSerial _iter436 in V)
          {
            await _iter436.WriteAsync(oprot, cancellationToken);
          }
          await oprot.WriteListEndAsync(cancellationToken);
        }
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
    if (!(that is FCellSerial other)) return false;
    if (ReferenceEquals(this, other)) return true;
    return ((__isset.c == other.__isset.c) && ((!__isset.c) || (global::System.Object.Equals(C, other.C))))
      && ((__isset.ts == other.__isset.ts) && ((!__isset.ts) || (global::System.Object.Equals(Ts, other.Ts))))
      && ((__isset.v == other.__isset.v) && ((!__isset.v) || (global::System.Object.Equals(V, other.V))));
  }

  public override int GetHashCode() {
    int hashcode = 157;
    unchecked {
      if((C != null) && __isset.c)
      {
        hashcode = (hashcode * 397) + C.GetHashCode();
      }
      if(__isset.ts)
      {
        hashcode = (hashcode * 397) + Ts.GetHashCode();
      }
      if((V != null) && __isset.v)
      {
        hashcode = (hashcode * 397) + V.GetHashCode();
      }
    }
    return hashcode;
  }

  public override string ToString()
  {
    var tmp437 = new StringBuilder("FCellSerial(");
    int tmp438 = 0;
    if((C != null) && __isset.c)
    {
      if(0 < tmp438++) { tmp437.Append(", "); }
      tmp437.Append("C: ");
      C.ToString(tmp437);
    }
    if(__isset.ts)
    {
      if(0 < tmp438++) { tmp437.Append(", "); }
      tmp437.Append("Ts: ");
      Ts.ToString(tmp437);
    }
    if((V != null) && __isset.v)
    {
      if(0 < tmp438++) { tmp437.Append(", "); }
      tmp437.Append("V: ");
      V.ToString(tmp437);
    }
    tmp437.Append(')');
    return tmp437.ToString();
  }
}

