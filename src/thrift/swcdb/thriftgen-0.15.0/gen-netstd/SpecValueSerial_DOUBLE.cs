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
/// The Specifications of DOUBLE Serial Value Field
/// </summary>
public partial class SpecValueSerial_DOUBLE : TBase
{
  private Comp _comp;
  private double _v;

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
  /// The double to match against the value field
  /// </summary>
  public double V
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

  public SpecValueSerial_DOUBLE()
  {
  }

  public SpecValueSerial_DOUBLE DeepCopy()
  {
    var tmp136 = new SpecValueSerial_DOUBLE();
    if(__isset.comp)
    {
      tmp136.Comp = this.Comp;
    }
    tmp136.__isset.comp = this.__isset.comp;
    if(__isset.v)
    {
      tmp136.V = this.V;
    }
    tmp136.__isset.v = this.__isset.v;
    return tmp136;
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
            if (field.Type == TType.Double)
            {
              V = await iprot.ReadDoubleAsync(cancellationToken);
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
      var tmp137 = new TStruct("SpecValueSerial_DOUBLE");
      await oprot.WriteStructBeginAsync(tmp137, cancellationToken);
      var tmp138 = new TField();
      if(__isset.comp)
      {
        tmp138.Name = "comp";
        tmp138.Type = TType.I32;
        tmp138.ID = 1;
        await oprot.WriteFieldBeginAsync(tmp138, cancellationToken);
        await oprot.WriteI32Async((int)Comp, cancellationToken);
        await oprot.WriteFieldEndAsync(cancellationToken);
      }
      if(__isset.v)
      {
        tmp138.Name = "v";
        tmp138.Type = TType.Double;
        tmp138.ID = 2;
        await oprot.WriteFieldBeginAsync(tmp138, cancellationToken);
        await oprot.WriteDoubleAsync(V, cancellationToken);
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
    if (!(that is SpecValueSerial_DOUBLE other)) return false;
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
    var tmp139 = new StringBuilder("SpecValueSerial_DOUBLE(");
    int tmp140 = 0;
    if(__isset.comp)
    {
      if(0 < tmp140++) { tmp139.Append(", "); }
      tmp139.Append("Comp: ");
      Comp.ToString(tmp139);
    }
    if(__isset.v)
    {
      if(0 < tmp140++) { tmp139.Append(", "); }
      tmp139.Append("V: ");
      V.ToString(tmp139);
    }
    tmp139.Append(')');
    return tmp139.ToString();
  }
}

