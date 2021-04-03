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
/// The Serial Value Specifications
/// </summary>
public partial class SpecValueSerial : TBase
{
  private Comp _comp;
  private List<SpecValueSerialField> _fields;

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
  /// The Serial Value Specifications to match against the SERIAL Cell value fields
  /// </summary>
  public List<SpecValueSerialField> Fields
  {
    get
    {
      return _fields;
    }
    set
    {
      __isset.fields = true;
      this._fields = value;
    }
  }


  public Isset __isset;
  public struct Isset
  {
    public bool comp;
    public bool fields;
  }

  public SpecValueSerial()
  {
  }

  public SpecValueSerial DeepCopy()
  {
    var tmp92 = new SpecValueSerial();
    if(__isset.comp)
    {
      tmp92.Comp = this.Comp;
    }
    tmp92.__isset.comp = this.__isset.comp;
    if((Fields != null) && __isset.fields)
    {
      tmp92.Fields = this.Fields.DeepCopy();
    }
    tmp92.__isset.fields = this.__isset.fields;
    return tmp92;
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
            if (field.Type == TType.List)
            {
              {
                TList _list93 = await iprot.ReadListBeginAsync(cancellationToken);
                Fields = new List<SpecValueSerialField>(_list93.Count);
                for(int _i94 = 0; _i94 < _list93.Count; ++_i94)
                {
                  SpecValueSerialField _elem95;
                  _elem95 = new SpecValueSerialField();
                  await _elem95.ReadAsync(iprot, cancellationToken);
                  Fields.Add(_elem95);
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
      var struc = new TStruct("SpecValueSerial");
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
      if((Fields != null) && __isset.fields)
      {
        field.Name = "fields";
        field.Type = TType.List;
        field.ID = 2;
        await oprot.WriteFieldBeginAsync(field, cancellationToken);
        {
          await oprot.WriteListBeginAsync(new TList(TType.Struct, Fields.Count), cancellationToken);
          foreach (SpecValueSerialField _iter96 in Fields)
          {
            await _iter96.WriteAsync(oprot, cancellationToken);
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
    if (!(that is SpecValueSerial other)) return false;
    if (ReferenceEquals(this, other)) return true;
    return ((__isset.comp == other.__isset.comp) && ((!__isset.comp) || (System.Object.Equals(Comp, other.Comp))))
      && ((__isset.fields == other.__isset.fields) && ((!__isset.fields) || (System.Object.Equals(Fields, other.Fields))));
  }

  public override int GetHashCode() {
    int hashcode = 157;
    unchecked {
      if(__isset.comp)
      {
        hashcode = (hashcode * 397) + Comp.GetHashCode();
      }
      if((Fields != null) && __isset.fields)
      {
        hashcode = (hashcode * 397) + Fields.GetHashCode();
      }
    }
    return hashcode;
  }

  public override string ToString()
  {
    var sb = new StringBuilder("SpecValueSerial(");
    int tmp97 = 0;
    if(__isset.comp)
    {
      if(0 < tmp97++) { sb.Append(", "); }
      sb.Append("Comp: ");
      Comp.ToString(sb);
    }
    if((Fields != null) && __isset.fields)
    {
      if(0 < tmp97++) { sb.Append(", "); }
      sb.Append("Fields: ");
      Fields.ToString(sb);
    }
    sb.Append(')');
    return sb.ToString();
  }
}

