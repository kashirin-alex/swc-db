/**
 * Autogenerated by Thrift Compiler (0.14.0)
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


/// <summary>
/// The Specifications of INT64 Serial Value Field
/// </summary>
public partial class SpecValueSerial_INT64 : TBase
{
  private Comp _comp;
  private long _v;

  /// <summary>
  /// Logical comparator to Apply
  /// 
  /// <seealso cref="global::.Comp"/>
  /// </summary>
  public Comp Comp
  {
    get
    {
      return _comp;
    }
    set
    {
      __isset.comp = true;
      this._comp = value;
    }
  }

  /// <summary>
  /// The int64 to match against the value field
  /// </summary>
  public long V
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
    public bool comp;
    public bool v;
  }

  public SpecValueSerial_INT64()
  {
  }

  public SpecValueSerial_INT64 DeepCopy()
  {
    var tmp70 = new SpecValueSerial_INT64();
    if(__isset.comp)
    {
      tmp70.Comp = this.Comp;
    }
    tmp70.__isset.comp = this.__isset.comp;
    if(__isset.v)
    {
      tmp70.V = this.V;
    }
    tmp70.__isset.v = this.__isset.v;
    return tmp70;
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
            if (field.Type == TType.I32)
            {
              Comp = (Comp)await iprot.ReadI32Async(cancellationToken);
            }
            else
            {
              await TProtocolUtil.SkipAsync(iprot, field.Type, cancellationToken);
            }
            break;
          case 2:
            if (field.Type == TType.I64)
            {
              V = await iprot.ReadI64Async(cancellationToken);
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
      var struc = new TStruct("SpecValueSerial_INT64");
      await oprot.WriteStructBeginAsync(struc, cancellationToken);
      var field = new TField();
      if(__isset.comp)
      {
        field.Name = "comp";
        field.Type = TType.I32;
        field.ID = 1;
        await oprot.WriteFieldBeginAsync(field, cancellationToken);
        await oprot.WriteI32Async((int)Comp, cancellationToken);
        await oprot.WriteFieldEndAsync(cancellationToken);
      }
      if(__isset.v)
      {
        field.Name = "v";
        field.Type = TType.I64;
        field.ID = 2;
        await oprot.WriteFieldBeginAsync(field, cancellationToken);
        await oprot.WriteI64Async(V, cancellationToken);
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
    if (!(that is SpecValueSerial_INT64 other)) return false;
    if (ReferenceEquals(this, other)) return true;
    return ((__isset.comp == other.__isset.comp) && ((!__isset.comp) || (System.Object.Equals(Comp, other.Comp))))
      && ((__isset.v == other.__isset.v) && ((!__isset.v) || (System.Object.Equals(V, other.V))));
  }

  public override int GetHashCode() {
    int hashcode = 157;
    unchecked {
      if(__isset.comp)
      {
        hashcode = (hashcode * 397) + Comp.GetHashCode();
      }
      if(__isset.v)
      {
        hashcode = (hashcode * 397) + V.GetHashCode();
      }
    }
    return hashcode;
  }

  public override string ToString()
  {
    var sb = new StringBuilder("SpecValueSerial_INT64(");
    int tmp71 = 0;
    if(__isset.comp)
    {
      if(0 < tmp71++) { sb.Append(", "); }
      sb.Append("Comp: ");
      Comp.ToString(sb);
    }
    if(__isset.v)
    {
      if(0 < tmp71++) { sb.Append(", "); }
      sb.Append("V: ");
      V.ToString(sb);
    }
    sb.Append(')');
    return sb.ToString();
  }
}
