/**
 * <auto-generated>
 * Autogenerated by Thrift Compiler (0.17.0)
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


#pragma warning disable IDE0079  // remove unnecessary pragmas
#pragma warning disable IDE0017  // object init can be simplified
#pragma warning disable IDE0028  // collection init can be simplified
#pragma warning disable IDE1006  // parts of the code use IDL spelling
#pragma warning disable CA1822   // empty DeepCopy() methods still non-static
#pragma warning disable IDE0083  // pattern matching "that is not SomeType" requires net5.0 but we still support earlier versions


/// <summary>
/// The Fraction Cell for results on Fraction of scan
/// </summary>
public partial class FCell : TBase
{
  private string _c;
  private long _ts;
  private byte[] _v;

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
  /// The Cell Value
  /// </summary>
  public byte[] V
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

  public FCell()
  {
  }

  public FCell DeepCopy()
  {
    var tmp517 = new FCell();
    if((C != null) && __isset.c)
    {
      tmp517.C = this.C;
    }
    tmp517.__isset.c = this.__isset.c;
    if(__isset.ts)
    {
      tmp517.Ts = this.Ts;
    }
    tmp517.__isset.ts = this.__isset.ts;
    if((V != null) && __isset.v)
    {
      tmp517.V = this.V.ToArray();
    }
    tmp517.__isset.v = this.__isset.v;
    return tmp517;
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
            if (field.Type == TType.String)
            {
              V = await iprot.ReadBinaryAsync(cancellationToken);
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
      var tmp518 = new TStruct("FCell");
      await oprot.WriteStructBeginAsync(tmp518, cancellationToken);
      var tmp519 = new TField();
      if((C != null) && __isset.c)
      {
        tmp519.Name = "c";
        tmp519.Type = TType.String;
        tmp519.ID = 1;
        await oprot.WriteFieldBeginAsync(tmp519, cancellationToken);
        await oprot.WriteStringAsync(C, cancellationToken);
        await oprot.WriteFieldEndAsync(cancellationToken);
      }
      if(__isset.ts)
      {
        tmp519.Name = "ts";
        tmp519.Type = TType.I64;
        tmp519.ID = 2;
        await oprot.WriteFieldBeginAsync(tmp519, cancellationToken);
        await oprot.WriteI64Async(Ts, cancellationToken);
        await oprot.WriteFieldEndAsync(cancellationToken);
      }
      if((V != null) && __isset.v)
      {
        tmp519.Name = "v";
        tmp519.Type = TType.String;
        tmp519.ID = 3;
        await oprot.WriteFieldBeginAsync(tmp519, cancellationToken);
        await oprot.WriteBinaryAsync(V, cancellationToken);
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
    if (!(that is FCell other)) return false;
    if (ReferenceEquals(this, other)) return true;
    return ((__isset.c == other.__isset.c) && ((!__isset.c) || (global::System.Object.Equals(C, other.C))))
      && ((__isset.ts == other.__isset.ts) && ((!__isset.ts) || (global::System.Object.Equals(Ts, other.Ts))))
      && ((__isset.v == other.__isset.v) && ((!__isset.v) || (TCollections.Equals(V, other.V))));
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
    var tmp520 = new StringBuilder("FCell(");
    int tmp521 = 0;
    if((C != null) && __isset.c)
    {
      if(0 < tmp521++) { tmp520.Append(", "); }
      tmp520.Append("C: ");
      C.ToString(tmp520);
    }
    if(__isset.ts)
    {
      if(0 < tmp521++) { tmp520.Append(", "); }
      tmp520.Append("Ts: ");
      Ts.ToString(tmp520);
    }
    if((V != null) && __isset.v)
    {
      if(0 < tmp521++) { tmp520.Append(", "); }
      tmp520.Append("V: ");
      V.ToString(tmp520);
    }
    tmp520.Append(')');
    return tmp520.ToString();
  }
}

