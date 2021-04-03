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
/// The Cells for results list of scan
/// </summary>
public partial class Cells : TBase
{
  private List<Cell> _cells;
  private List<CellSerial> _serial_cells;

  /// <summary>
  /// The Cells, defined as Cell items in a list-container
  /// </summary>
  public List<Cell> Cells_
  {
    get
    {
      return _cells;
    }
    set
    {
      __isset.cells = true;
      this._cells = value;
    }
  }

  /// <summary>
  /// The Serial Cells, defined as CellSerial items in a list-container
  /// </summary>
  public List<CellSerial> Serial_cells
  {
    get
    {
      return _serial_cells;
    }
    set
    {
      __isset.serial_cells = true;
      this._serial_cells = value;
    }
  }


  public Isset __isset;
  public struct Isset
  {
    public bool cells;
    public bool serial_cells;
  }

  public Cells()
  {
  }

  public Cells DeepCopy()
  {
    var tmp182 = new Cells();
    if((Cells_ != null) && __isset.cells)
    {
      tmp182.Cells_ = this.Cells_.DeepCopy();
    }
    tmp182.__isset.cells = this.__isset.cells;
    if((Serial_cells != null) && __isset.serial_cells)
    {
      tmp182.Serial_cells = this.Serial_cells.DeepCopy();
    }
    tmp182.__isset.serial_cells = this.__isset.serial_cells;
    return tmp182;
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
                TList _list183 = await iprot.ReadListBeginAsync(cancellationToken);
                Cells_ = new List<Cell>(_list183.Count);
                for(int _i184 = 0; _i184 < _list183.Count; ++_i184)
                {
                  Cell _elem185;
                  _elem185 = new Cell();
                  await _elem185.ReadAsync(iprot, cancellationToken);
                  Cells_.Add(_elem185);
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
                TList _list186 = await iprot.ReadListBeginAsync(cancellationToken);
                Serial_cells = new List<CellSerial>(_list186.Count);
                for(int _i187 = 0; _i187 < _list186.Count; ++_i187)
                {
                  CellSerial _elem188;
                  _elem188 = new CellSerial();
                  await _elem188.ReadAsync(iprot, cancellationToken);
                  Serial_cells.Add(_elem188);
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
      var struc = new TStruct("Cells");
      await oprot.WriteStructBeginAsync(struc, cancellationToken);
      var field = new TField();
      if((Cells_ != null) && __isset.cells)
      {
        field.Name = "cells";
        field.Type = TType.List;
        field.ID = 1;
        await oprot.WriteFieldBeginAsync(field, cancellationToken);
        {
          await oprot.WriteListBeginAsync(new TList(TType.Struct, Cells_.Count), cancellationToken);
          foreach (Cell _iter189 in Cells_)
          {
            await _iter189.WriteAsync(oprot, cancellationToken);
          }
          await oprot.WriteListEndAsync(cancellationToken);
        }
        await oprot.WriteFieldEndAsync(cancellationToken);
      }
      if((Serial_cells != null) && __isset.serial_cells)
      {
        field.Name = "serial_cells";
        field.Type = TType.List;
        field.ID = 2;
        await oprot.WriteFieldBeginAsync(field, cancellationToken);
        {
          await oprot.WriteListBeginAsync(new TList(TType.Struct, Serial_cells.Count), cancellationToken);
          foreach (CellSerial _iter190 in Serial_cells)
          {
            await _iter190.WriteAsync(oprot, cancellationToken);
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
    if (!(that is Cells other)) return false;
    if (ReferenceEquals(this, other)) return true;
    return ((__isset.cells == other.__isset.cells) && ((!__isset.cells) || (TCollections.Equals(Cells_, other.Cells_))))
      && ((__isset.serial_cells == other.__isset.serial_cells) && ((!__isset.serial_cells) || (TCollections.Equals(Serial_cells, other.Serial_cells))));
  }

  public override int GetHashCode() {
    int hashcode = 157;
    unchecked {
      if((Cells_ != null) && __isset.cells)
      {
        hashcode = (hashcode * 397) + TCollections.GetHashCode(Cells_);
      }
      if((Serial_cells != null) && __isset.serial_cells)
      {
        hashcode = (hashcode * 397) + TCollections.GetHashCode(Serial_cells);
      }
    }
    return hashcode;
  }

  public override string ToString()
  {
    var sb = new StringBuilder("Cells(");
    int tmp191 = 0;
    if((Cells_ != null) && __isset.cells)
    {
      if(0 < tmp191++) { sb.Append(", "); }
      sb.Append("Cells_: ");
      Cells_.ToString(sb);
    }
    if((Serial_cells != null) && __isset.serial_cells)
    {
      if(0 < tmp191++) { sb.Append(", "); }
      sb.Append("Serial_cells: ");
      Serial_cells.ToString(sb);
    }
    sb.Append(')');
    return sb.ToString();
  }
}

