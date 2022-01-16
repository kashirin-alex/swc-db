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
/// The Value specs for an Updating Interval of 'updating' in SpecIntervalSerial
/// </summary>
public partial class SpecIntervalUpdateSerial : TBase
{
  private long _ts;
  private List<CellValueSerial> _v;
  private EncodingType _encoder;

  /// <summary>
  /// The timestamp for the updated cell NULL: MIN_INT64-1, AUTO:MIN_INT64-1
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
  /// The value for the updated cell
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

  /// <summary>
  /// Optionally the Cell Value Encoding Type: ZLIB/SNAPPY/ZSTD
  /// 
  /// <seealso cref="global::.EncodingType"/>
  /// </summary>
  public EncodingType Encoder
  {
    get
    {
      return _encoder;
    }
    set
    {
      __isset.encoder = true;
      this._encoder = value;
    }
  }


  public Isset __isset;
  public struct Isset
  {
    public bool ts;
    public bool v;
    public bool encoder;
  }

  public SpecIntervalUpdateSerial()
  {
  }

  public SpecIntervalUpdateSerial DeepCopy()
  {
    var tmp88 = new SpecIntervalUpdateSerial();
    if(__isset.ts)
    {
      tmp88.Ts = this.Ts;
    }
    tmp88.__isset.ts = this.__isset.ts;
    if((V != null) && __isset.v)
    {
      tmp88.V = this.V.DeepCopy();
    }
    tmp88.__isset.v = this.__isset.v;
    if(__isset.encoder)
    {
      tmp88.Encoder = this.Encoder;
    }
    tmp88.__isset.encoder = this.__isset.encoder;
    return tmp88;
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
            if (field.Type == TType.I64)
            {
              Ts = await iprot.ReadI64Async(cancellationToken);
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
                TList _list89 = await iprot.ReadListBeginAsync(cancellationToken);
                V = new List<CellValueSerial>(_list89.Count);
                for(int _i90 = 0; _i90 < _list89.Count; ++_i90)
                {
                  CellValueSerial _elem91;
                  _elem91 = new CellValueSerial();
                  await _elem91.ReadAsync(iprot, cancellationToken);
                  V.Add(_elem91);
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
            if (field.Type == TType.I32)
            {
              Encoder = (EncodingType)await iprot.ReadI32Async(cancellationToken);
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
      var tmp92 = new TStruct("SpecIntervalUpdateSerial");
      await oprot.WriteStructBeginAsync(tmp92, cancellationToken);
      var tmp93 = new TField();
      if(__isset.ts)
      {
        tmp93.Name = "ts";
        tmp93.Type = TType.I64;
        tmp93.ID = 1;
        await oprot.WriteFieldBeginAsync(tmp93, cancellationToken);
        await oprot.WriteI64Async(Ts, cancellationToken);
        await oprot.WriteFieldEndAsync(cancellationToken);
      }
      if((V != null) && __isset.v)
      {
        tmp93.Name = "v";
        tmp93.Type = TType.List;
        tmp93.ID = 2;
        await oprot.WriteFieldBeginAsync(tmp93, cancellationToken);
        {
          await oprot.WriteListBeginAsync(new TList(TType.Struct, V.Count), cancellationToken);
          foreach (CellValueSerial _iter94 in V)
          {
            await _iter94.WriteAsync(oprot, cancellationToken);
          }
          await oprot.WriteListEndAsync(cancellationToken);
        }
        await oprot.WriteFieldEndAsync(cancellationToken);
      }
      if(__isset.encoder)
      {
        tmp93.Name = "encoder";
        tmp93.Type = TType.I32;
        tmp93.ID = 3;
        await oprot.WriteFieldBeginAsync(tmp93, cancellationToken);
        await oprot.WriteI32Async((int)Encoder, cancellationToken);
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
    if (!(that is SpecIntervalUpdateSerial other)) return false;
    if (ReferenceEquals(this, other)) return true;
    return ((__isset.ts == other.__isset.ts) && ((!__isset.ts) || (System.Object.Equals(Ts, other.Ts))))
      && ((__isset.v == other.__isset.v) && ((!__isset.v) || (System.Object.Equals(V, other.V))))
      && ((__isset.encoder == other.__isset.encoder) && ((!__isset.encoder) || (System.Object.Equals(Encoder, other.Encoder))));
  }

  public override int GetHashCode() {
    int hashcode = 157;
    unchecked {
      if(__isset.ts)
      {
        hashcode = (hashcode * 397) + Ts.GetHashCode();
      }
      if((V != null) && __isset.v)
      {
        hashcode = (hashcode * 397) + V.GetHashCode();
      }
      if(__isset.encoder)
      {
        hashcode = (hashcode * 397) + Encoder.GetHashCode();
      }
    }
    return hashcode;
  }

  public override string ToString()
  {
    var tmp95 = new StringBuilder("SpecIntervalUpdateSerial(");
    int tmp96 = 0;
    if(__isset.ts)
    {
      if(0 < tmp96++) { tmp95.Append(", "); }
      tmp95.Append("Ts: ");
      Ts.ToString(tmp95);
    }
    if((V != null) && __isset.v)
    {
      if(0 < tmp96++) { tmp95.Append(", "); }
      tmp95.Append("V: ");
      V.ToString(tmp95);
    }
    if(__isset.encoder)
    {
      if(0 < tmp96++) { tmp95.Append(", "); }
      tmp95.Append("Encoder: ");
      Encoder.ToString(tmp95);
    }
    tmp95.Append(')');
    return tmp95.ToString();
  }
}

