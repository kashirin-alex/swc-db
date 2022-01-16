/**
 * Autogenerated by Thrift Compiler (0.15.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
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
#pragma warning disable IDE1006  // parts of the code use IDL spelling
#pragma warning disable IDE0083  // pattern matching "that is not SomeType" requires net5.0 but we still support earlier versions


/// <summary>
/// The Column Cell for results on Columns of scan
/// </summary>
public partial class CCell : TBase
{
  private List<byte[]> _k;
  private long _ts;
  private byte[] _v;

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
    public bool k;
    public bool ts;
    public bool v;
  }

  public CCell()
  {
  }

  public CCell DeepCopy()
  {
    var tmp308 = new CCell();
    if((K != null) && __isset.k)
    {
      tmp308.K = this.K.DeepCopy();
    }
    tmp308.__isset.k = this.__isset.k;
    if(__isset.ts)
    {
      tmp308.Ts = this.Ts;
    }
    tmp308.__isset.ts = this.__isset.ts;
    if((V != null) && __isset.v)
    {
      tmp308.V = this.V.ToArray();
    }
    tmp308.__isset.v = this.__isset.v;
    return tmp308;
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
                TList _list309 = await iprot.ReadListBeginAsync(cancellationToken);
                K = new List<byte[]>(_list309.Count);
                for(int _i310 = 0; _i310 < _list309.Count; ++_i310)
                {
                  byte[] _elem311;
                  _elem311 = await iprot.ReadBinaryAsync(cancellationToken);
                  K.Add(_elem311);
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
      var tmp312 = new TStruct("CCell");
      await oprot.WriteStructBeginAsync(tmp312, cancellationToken);
      var tmp313 = new TField();
      if((K != null) && __isset.k)
      {
        tmp313.Name = "k";
        tmp313.Type = TType.List;
        tmp313.ID = 1;
        await oprot.WriteFieldBeginAsync(tmp313, cancellationToken);
        {
          await oprot.WriteListBeginAsync(new TList(TType.String, K.Count), cancellationToken);
          foreach (byte[] _iter314 in K)
          {
            await oprot.WriteBinaryAsync(_iter314, cancellationToken);
          }
          await oprot.WriteListEndAsync(cancellationToken);
        }
        await oprot.WriteFieldEndAsync(cancellationToken);
      }
      if(__isset.ts)
      {
        tmp313.Name = "ts";
        tmp313.Type = TType.I64;
        tmp313.ID = 2;
        await oprot.WriteFieldBeginAsync(tmp313, cancellationToken);
        await oprot.WriteI64Async(Ts, cancellationToken);
        await oprot.WriteFieldEndAsync(cancellationToken);
      }
      if((V != null) && __isset.v)
      {
        tmp313.Name = "v";
        tmp313.Type = TType.String;
        tmp313.ID = 3;
        await oprot.WriteFieldBeginAsync(tmp313, cancellationToken);
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
    if (!(that is CCell other)) return false;
    if (ReferenceEquals(this, other)) return true;
    return ((__isset.k == other.__isset.k) && ((!__isset.k) || (System.Object.Equals(K, other.K))))
      && ((__isset.ts == other.__isset.ts) && ((!__isset.ts) || (System.Object.Equals(Ts, other.Ts))))
      && ((__isset.v == other.__isset.v) && ((!__isset.v) || (TCollections.Equals(V, other.V))));
  }

  public override int GetHashCode() {
    int hashcode = 157;
    unchecked {
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
    var tmp315 = new StringBuilder("CCell(");
    int tmp316 = 0;
    if((K != null) && __isset.k)
    {
      if(0 < tmp316++) { tmp315.Append(", "); }
      tmp315.Append("K: ");
      K.ToString(tmp315);
    }
    if(__isset.ts)
    {
      if(0 < tmp316++) { tmp315.Append(", "); }
      tmp315.Append("Ts: ");
      Ts.ToString(tmp315);
    }
    if((V != null) && __isset.v)
    {
      if(0 < tmp316++) { tmp315.Append(", "); }
      tmp315.Append("V: ");
      V.ToString(tmp315);
    }
    tmp315.Append(')');
    return tmp315.ToString();
  }
}

