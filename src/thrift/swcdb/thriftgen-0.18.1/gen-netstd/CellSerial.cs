/**
 * <auto-generated>
 * Autogenerated by Thrift Compiler (0.18.1)
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
/// The Serial Cell for results list of scan
/// </summary>
public partial class CellSerial : TBase
{
  private string _c;
  private List<byte[]> _k;
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
  /// The Cell Key
  /// </summary>
  public List<byte[]> K
  {
    get
    {
      return _k;
    }
    set
    {
      __isset.k = true;
      this._k = value;
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
    public bool k;
    public bool ts;
    public bool v;
  }

  public CellSerial()
  {
  }

  public CellSerial DeepCopy()
  {
    var tmp399 = new CellSerial();
    if((C != null) && __isset.c)
    {
      tmp399.C = this.C;
    }
    tmp399.__isset.c = this.__isset.c;
    if((K != null) && __isset.k)
    {
      tmp399.K = this.K.DeepCopy();
    }
    tmp399.__isset.k = this.__isset.k;
    if(__isset.ts)
    {
      tmp399.Ts = this.Ts;
    }
    tmp399.__isset.ts = this.__isset.ts;
    if((V != null) && __isset.v)
    {
      tmp399.V = this.V.DeepCopy();
    }
    tmp399.__isset.v = this.__isset.v;
    return tmp399;
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
            if (field.Type == TType.List)
            {
              {
                var _list400 = await iprot.ReadListBeginAsync(cancellationToken);
                K = new List<byte[]>(_list400.Count);
                for(int _i401 = 0; _i401 < _list400.Count; ++_i401)
                {
                  byte[] _elem402;
                  _elem402 = await iprot.ReadBinaryAsync(cancellationToken);
                  K.Add(_elem402);
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
            if (field.Type == TType.I64)
            {
              Ts = await iprot.ReadI64Async(cancellationToken);
            }
            else
            {
              await TProtocolUtil.SkipAsync(iprot, field.Type, cancellationToken);
            }
            break;
          case 4:
            if (field.Type == TType.List)
            {
              {
                var _list403 = await iprot.ReadListBeginAsync(cancellationToken);
                V = new List<CellValueSerial>(_list403.Count);
                for(int _i404 = 0; _i404 < _list403.Count; ++_i404)
                {
                  CellValueSerial _elem405;
                  _elem405 = new CellValueSerial();
                  await _elem405.ReadAsync(iprot, cancellationToken);
                  V.Add(_elem405);
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
      var tmp406 = new TStruct("CellSerial");
      await oprot.WriteStructBeginAsync(tmp406, cancellationToken);
      var tmp407 = new TField();
      if((C != null) && __isset.c)
      {
        tmp407.Name = "c";
        tmp407.Type = TType.String;
        tmp407.ID = 1;
        await oprot.WriteFieldBeginAsync(tmp407, cancellationToken);
        await oprot.WriteStringAsync(C, cancellationToken);
        await oprot.WriteFieldEndAsync(cancellationToken);
      }
      if((K != null) && __isset.k)
      {
        tmp407.Name = "k";
        tmp407.Type = TType.List;
        tmp407.ID = 2;
        await oprot.WriteFieldBeginAsync(tmp407, cancellationToken);
        await oprot.WriteListBeginAsync(new TList(TType.String, K.Count), cancellationToken);
        foreach (byte[] _iter408 in K)
        {
          await oprot.WriteBinaryAsync(_iter408, cancellationToken);
        }
        await oprot.WriteListEndAsync(cancellationToken);
        await oprot.WriteFieldEndAsync(cancellationToken);
      }
      if(__isset.ts)
      {
        tmp407.Name = "ts";
        tmp407.Type = TType.I64;
        tmp407.ID = 3;
        await oprot.WriteFieldBeginAsync(tmp407, cancellationToken);
        await oprot.WriteI64Async(Ts, cancellationToken);
        await oprot.WriteFieldEndAsync(cancellationToken);
      }
      if((V != null) && __isset.v)
      {
        tmp407.Name = "v";
        tmp407.Type = TType.List;
        tmp407.ID = 4;
        await oprot.WriteFieldBeginAsync(tmp407, cancellationToken);
        await oprot.WriteListBeginAsync(new TList(TType.Struct, V.Count), cancellationToken);
        foreach (CellValueSerial _iter409 in V)
        {
          await _iter409.WriteAsync(oprot, cancellationToken);
        }
        await oprot.WriteListEndAsync(cancellationToken);
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
    if (!(that is CellSerial other)) return false;
    if (ReferenceEquals(this, other)) return true;
    return ((__isset.c == other.__isset.c) && ((!__isset.c) || (global::System.Object.Equals(C, other.C))))
      && ((__isset.k == other.__isset.k) && ((!__isset.k) || (global::System.Object.Equals(K, other.K))))
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
      if((K != null) && __isset.k)
      {
        hashcode = (hashcode * 397) + K.GetHashCode();
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
    var tmp410 = new StringBuilder("CellSerial(");
    int tmp411 = 0;
    if((C != null) && __isset.c)
    {
      if(0 < tmp411++) { tmp410.Append(", "); }
      tmp410.Append("C: ");
      C.ToString(tmp410);
    }
    if((K != null) && __isset.k)
    {
      if(0 < tmp411++) { tmp410.Append(", "); }
      tmp410.Append("K: ");
      K.ToString(tmp410);
    }
    if(__isset.ts)
    {
      if(0 < tmp411++) { tmp410.Append(", "); }
      tmp410.Append("Ts: ");
      Ts.ToString(tmp410);
    }
    if((V != null) && __isset.v)
    {
      if(0 < tmp411++) { tmp410.Append(", "); }
      tmp410.Append("V: ");
      V.ToString(tmp410);
    }
    tmp410.Append(')');
    return tmp410.ToString();
  }
}
