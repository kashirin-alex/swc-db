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


#pragma warning disable IDE0079  // remove unnecessary pragmas
#pragma warning disable IDE0017  // object init can be simplified
#pragma warning disable IDE0028  // collection init can be simplified
#pragma warning disable IDE1006  // parts of the code use IDL spelling
#pragma warning disable CA1822   // empty DeepCopy() methods still non-static
#pragma warning disable IDE0083  // pattern matching "that is not SomeType" requires net5.0 but we still support earlier versions

public static class ServiceExtensions
{
  public static bool Equals(this Dictionary<byte[], FCells> instance, object that)
  {
    if (!(that is Dictionary<byte[], FCells> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this Dictionary<byte[], FCells> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static Dictionary<byte[], FCells> DeepCopy(this Dictionary<byte[], FCells> source)
  {
    if (source == null)
      return null;

    var tmp1230 = new Dictionary<byte[], FCells>(source.Count);
    foreach (var pair in source)
      tmp1230.Add((pair.Key != null) ? pair.Key.ToArray() : null, (pair.Value != null) ? pair.Value.DeepCopy() : null);
    return tmp1230;
  }


  public static bool Equals(this Dictionary<long, List<UCellCounter>> instance, object that)
  {
    if (!(that is Dictionary<long, List<UCellCounter>> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this Dictionary<long, List<UCellCounter>> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static Dictionary<long, List<UCellCounter>> DeepCopy(this Dictionary<long, List<UCellCounter>> source)
  {
    if (source == null)
      return null;

    var tmp1231 = new Dictionary<long, List<UCellCounter>>(source.Count);
    foreach (var pair in source)
      tmp1231.Add(pair.Key, (pair.Value != null) ? pair.Value.DeepCopy() : null);
    return tmp1231;
  }


  public static bool Equals(this Dictionary<long, List<UCellPlain>> instance, object that)
  {
    if (!(that is Dictionary<long, List<UCellPlain>> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this Dictionary<long, List<UCellPlain>> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static Dictionary<long, List<UCellPlain>> DeepCopy(this Dictionary<long, List<UCellPlain>> source)
  {
    if (source == null)
      return null;

    var tmp1232 = new Dictionary<long, List<UCellPlain>>(source.Count);
    foreach (var pair in source)
      tmp1232.Add(pair.Key, (pair.Value != null) ? pair.Value.DeepCopy() : null);
    return tmp1232;
  }


  public static bool Equals(this Dictionary<long, List<UCellSerial>> instance, object that)
  {
    if (!(that is Dictionary<long, List<UCellSerial>> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this Dictionary<long, List<UCellSerial>> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static Dictionary<long, List<UCellSerial>> DeepCopy(this Dictionary<long, List<UCellSerial>> source)
  {
    if (source == null)
      return null;

    var tmp1233 = new Dictionary<long, List<UCellSerial>>(source.Count);
    foreach (var pair in source)
      tmp1233.Add(pair.Key, (pair.Value != null) ? pair.Value.DeepCopy() : null);
    return tmp1233;
  }


  public static bool Equals(this Dictionary<string, cCells> instance, object that)
  {
    if (!(that is Dictionary<string, cCells> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this Dictionary<string, cCells> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static Dictionary<string, cCells> DeepCopy(this Dictionary<string, cCells> source)
  {
    if (source == null)
      return null;

    var tmp1234 = new Dictionary<string, cCells>(source.Count);
    foreach (var pair in source)
      tmp1234.Add((pair.Key != null) ? pair.Key : null, (pair.Value != null) ? pair.Value.DeepCopy() : null);
    return tmp1234;
  }


  public static bool Equals(this List<CCellCounter> instance, object that)
  {
    if (!(that is List<CCellCounter> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<CCellCounter> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<CCellCounter> DeepCopy(this List<CCellCounter> source)
  {
    if (source == null)
      return null;

    var tmp1235 = new List<CCellCounter>(source.Count);
    foreach (var elem in source)
      tmp1235.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1235;
  }


  public static bool Equals(this List<CCellPlain> instance, object that)
  {
    if (!(that is List<CCellPlain> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<CCellPlain> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<CCellPlain> DeepCopy(this List<CCellPlain> source)
  {
    if (source == null)
      return null;

    var tmp1236 = new List<CCellPlain>(source.Count);
    foreach (var elem in source)
      tmp1236.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1236;
  }


  public static bool Equals(this List<CCellSerial> instance, object that)
  {
    if (!(that is List<CCellSerial> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<CCellSerial> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<CCellSerial> DeepCopy(this List<CCellSerial> source)
  {
    if (source == null)
      return null;

    var tmp1237 = new List<CCellSerial>(source.Count);
    foreach (var elem in source)
      tmp1237.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1237;
  }


  public static bool Equals(this List<CellCounter> instance, object that)
  {
    if (!(that is List<CellCounter> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<CellCounter> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<CellCounter> DeepCopy(this List<CellCounter> source)
  {
    if (source == null)
      return null;

    var tmp1238 = new List<CellCounter>(source.Count);
    foreach (var elem in source)
      tmp1238.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1238;
  }


  public static bool Equals(this List<CellPlain> instance, object that)
  {
    if (!(that is List<CellPlain> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<CellPlain> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<CellPlain> DeepCopy(this List<CellPlain> source)
  {
    if (source == null)
      return null;

    var tmp1239 = new List<CellPlain>(source.Count);
    foreach (var elem in source)
      tmp1239.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1239;
  }


  public static bool Equals(this List<CellSerial> instance, object that)
  {
    if (!(that is List<CellSerial> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<CellSerial> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<CellSerial> DeepCopy(this List<CellSerial> source)
  {
    if (source == null)
      return null;

    var tmp1240 = new List<CellSerial>(source.Count);
    foreach (var elem in source)
      tmp1240.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1240;
  }


  public static bool Equals(this List<CellValueSerial> instance, object that)
  {
    if (!(that is List<CellValueSerial> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<CellValueSerial> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<CellValueSerial> DeepCopy(this List<CellValueSerial> source)
  {
    if (source == null)
      return null;

    var tmp1241 = new List<CellValueSerial>(source.Count);
    foreach (var elem in source)
      tmp1241.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1241;
  }


  public static bool Equals(this List<CellValueSerialOp> instance, object that)
  {
    if (!(that is List<CellValueSerialOp> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<CellValueSerialOp> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<CellValueSerialOp> DeepCopy(this List<CellValueSerialOp> source)
  {
    if (source == null)
      return null;

    var tmp1242 = new List<CellValueSerialOp>(source.Count);
    foreach (var elem in source)
      tmp1242.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1242;
  }


  public static bool Equals(this List<CompactResult> instance, object that)
  {
    if (!(that is List<CompactResult> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<CompactResult> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<CompactResult> DeepCopy(this List<CompactResult> source)
  {
    if (source == null)
      return null;

    var tmp1243 = new List<CompactResult>(source.Count);
    foreach (var elem in source)
      tmp1243.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1243;
  }


  public static bool Equals(this List<FCellCounter> instance, object that)
  {
    if (!(that is List<FCellCounter> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<FCellCounter> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<FCellCounter> DeepCopy(this List<FCellCounter> source)
  {
    if (source == null)
      return null;

    var tmp1244 = new List<FCellCounter>(source.Count);
    foreach (var elem in source)
      tmp1244.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1244;
  }


  public static bool Equals(this List<FCellPlain> instance, object that)
  {
    if (!(that is List<FCellPlain> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<FCellPlain> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<FCellPlain> DeepCopy(this List<FCellPlain> source)
  {
    if (source == null)
      return null;

    var tmp1245 = new List<FCellPlain>(source.Count);
    foreach (var elem in source)
      tmp1245.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1245;
  }


  public static bool Equals(this List<FCellSerial> instance, object that)
  {
    if (!(that is List<FCellSerial> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<FCellSerial> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<FCellSerial> DeepCopy(this List<FCellSerial> source)
  {
    if (source == null)
      return null;

    var tmp1246 = new List<FCellSerial>(source.Count);
    foreach (var elem in source)
      tmp1246.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1246;
  }


  public static bool Equals(this List<FU_BYTES> instance, object that)
  {
    if (!(that is List<FU_BYTES> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<FU_BYTES> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<FU_BYTES> DeepCopy(this List<FU_BYTES> source)
  {
    if (source == null)
      return null;

    var tmp1247 = new List<FU_BYTES>(source.Count);
    foreach (var elem in source)
      tmp1247.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1247;
  }


  public static bool Equals(this List<FU_INT64> instance, object that)
  {
    if (!(that is List<FU_INT64> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<FU_INT64> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<FU_INT64> DeepCopy(this List<FU_INT64> source)
  {
    if (source == null)
      return null;

    var tmp1248 = new List<FU_INT64>(source.Count);
    foreach (var elem in source)
      tmp1248.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1248;
  }


  public static bool Equals(this List<KCellCounter> instance, object that)
  {
    if (!(that is List<KCellCounter> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<KCellCounter> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<KCellCounter> DeepCopy(this List<KCellCounter> source)
  {
    if (source == null)
      return null;

    var tmp1249 = new List<KCellCounter>(source.Count);
    foreach (var elem in source)
      tmp1249.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1249;
  }


  public static bool Equals(this List<KCellPlain> instance, object that)
  {
    if (!(that is List<KCellPlain> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<KCellPlain> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<KCellPlain> DeepCopy(this List<KCellPlain> source)
  {
    if (source == null)
      return null;

    var tmp1250 = new List<KCellPlain>(source.Count);
    foreach (var elem in source)
      tmp1250.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1250;
  }


  public static bool Equals(this List<KCellSerial> instance, object that)
  {
    if (!(that is List<KCellSerial> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<KCellSerial> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<KCellSerial> DeepCopy(this List<KCellSerial> source)
  {
    if (source == null)
      return null;

    var tmp1251 = new List<KCellSerial>(source.Count);
    foreach (var elem in source)
      tmp1251.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1251;
  }


  public static bool Equals(this List<Schema> instance, object that)
  {
    if (!(that is List<Schema> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<Schema> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<Schema> DeepCopy(this List<Schema> source)
  {
    if (source == null)
      return null;

    var tmp1252 = new List<Schema>(source.Count);
    foreach (var elem in source)
      tmp1252.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1252;
  }


  public static bool Equals(this List<SchemaPattern> instance, object that)
  {
    if (!(that is List<SchemaPattern> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<SchemaPattern> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<SchemaPattern> DeepCopy(this List<SchemaPattern> source)
  {
    if (source == null)
      return null;

    var tmp1253 = new List<SchemaPattern>(source.Count);
    foreach (var elem in source)
      tmp1253.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1253;
  }


  public static bool Equals(this List<SpecColumnCounter> instance, object that)
  {
    if (!(that is List<SpecColumnCounter> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<SpecColumnCounter> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<SpecColumnCounter> DeepCopy(this List<SpecColumnCounter> source)
  {
    if (source == null)
      return null;

    var tmp1254 = new List<SpecColumnCounter>(source.Count);
    foreach (var elem in source)
      tmp1254.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1254;
  }


  public static bool Equals(this List<SpecColumnPlain> instance, object that)
  {
    if (!(that is List<SpecColumnPlain> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<SpecColumnPlain> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<SpecColumnPlain> DeepCopy(this List<SpecColumnPlain> source)
  {
    if (source == null)
      return null;

    var tmp1255 = new List<SpecColumnPlain>(source.Count);
    foreach (var elem in source)
      tmp1255.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1255;
  }


  public static bool Equals(this List<SpecColumnSerial> instance, object that)
  {
    if (!(that is List<SpecColumnSerial> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<SpecColumnSerial> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<SpecColumnSerial> DeepCopy(this List<SpecColumnSerial> source)
  {
    if (source == null)
      return null;

    var tmp1256 = new List<SpecColumnSerial>(source.Count);
    foreach (var elem in source)
      tmp1256.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1256;
  }


  public static bool Equals(this List<SpecFraction> instance, object that)
  {
    if (!(that is List<SpecFraction> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<SpecFraction> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<SpecFraction> DeepCopy(this List<SpecFraction> source)
  {
    if (source == null)
      return null;

    var tmp1257 = new List<SpecFraction>(source.Count);
    foreach (var elem in source)
      tmp1257.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1257;
  }


  public static bool Equals(this List<SpecIntervalCounter> instance, object that)
  {
    if (!(that is List<SpecIntervalCounter> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<SpecIntervalCounter> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<SpecIntervalCounter> DeepCopy(this List<SpecIntervalCounter> source)
  {
    if (source == null)
      return null;

    var tmp1258 = new List<SpecIntervalCounter>(source.Count);
    foreach (var elem in source)
      tmp1258.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1258;
  }


  public static bool Equals(this List<SpecIntervalPlain> instance, object that)
  {
    if (!(that is List<SpecIntervalPlain> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<SpecIntervalPlain> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<SpecIntervalPlain> DeepCopy(this List<SpecIntervalPlain> source)
  {
    if (source == null)
      return null;

    var tmp1259 = new List<SpecIntervalPlain>(source.Count);
    foreach (var elem in source)
      tmp1259.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1259;
  }


  public static bool Equals(this List<SpecIntervalSerial> instance, object that)
  {
    if (!(that is List<SpecIntervalSerial> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<SpecIntervalSerial> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<SpecIntervalSerial> DeepCopy(this List<SpecIntervalSerial> source)
  {
    if (source == null)
      return null;

    var tmp1260 = new List<SpecIntervalSerial>(source.Count);
    foreach (var elem in source)
      tmp1260.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1260;
  }


  public static bool Equals(this List<SpecKeyInterval> instance, object that)
  {
    if (!(that is List<SpecKeyInterval> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<SpecKeyInterval> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<SpecKeyInterval> DeepCopy(this List<SpecKeyInterval> source)
  {
    if (source == null)
      return null;

    var tmp1261 = new List<SpecKeyInterval>(source.Count);
    foreach (var elem in source)
      tmp1261.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1261;
  }


  public static bool Equals(this List<SpecValueCounter> instance, object that)
  {
    if (!(that is List<SpecValueCounter> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<SpecValueCounter> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<SpecValueCounter> DeepCopy(this List<SpecValueCounter> source)
  {
    if (source == null)
      return null;

    var tmp1262 = new List<SpecValueCounter>(source.Count);
    foreach (var elem in source)
      tmp1262.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1262;
  }


  public static bool Equals(this List<SpecValuePlain> instance, object that)
  {
    if (!(that is List<SpecValuePlain> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<SpecValuePlain> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<SpecValuePlain> DeepCopy(this List<SpecValuePlain> source)
  {
    if (source == null)
      return null;

    var tmp1263 = new List<SpecValuePlain>(source.Count);
    foreach (var elem in source)
      tmp1263.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1263;
  }


  public static bool Equals(this List<SpecValueSerial> instance, object that)
  {
    if (!(that is List<SpecValueSerial> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<SpecValueSerial> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<SpecValueSerial> DeepCopy(this List<SpecValueSerial> source)
  {
    if (source == null)
      return null;

    var tmp1264 = new List<SpecValueSerial>(source.Count);
    foreach (var elem in source)
      tmp1264.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1264;
  }


  public static bool Equals(this List<SpecValueSerialField> instance, object that)
  {
    if (!(that is List<SpecValueSerialField> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<SpecValueSerialField> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<SpecValueSerialField> DeepCopy(this List<SpecValueSerialField> source)
  {
    if (source == null)
      return null;

    var tmp1265 = new List<SpecValueSerialField>(source.Count);
    foreach (var elem in source)
      tmp1265.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1265;
  }


  public static bool Equals(this List<SpecValueSerial_BYTES> instance, object that)
  {
    if (!(that is List<SpecValueSerial_BYTES> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<SpecValueSerial_BYTES> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<SpecValueSerial_BYTES> DeepCopy(this List<SpecValueSerial_BYTES> source)
  {
    if (source == null)
      return null;

    var tmp1266 = new List<SpecValueSerial_BYTES>(source.Count);
    foreach (var elem in source)
      tmp1266.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1266;
  }


  public static bool Equals(this List<SpecValueSerial_INT64> instance, object that)
  {
    if (!(that is List<SpecValueSerial_INT64> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<SpecValueSerial_INT64> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<SpecValueSerial_INT64> DeepCopy(this List<SpecValueSerial_INT64> source)
  {
    if (source == null)
      return null;

    var tmp1267 = new List<SpecValueSerial_INT64>(source.Count);
    foreach (var elem in source)
      tmp1267.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1267;
  }


  public static bool Equals(this List<UCellCounter> instance, object that)
  {
    if (!(that is List<UCellCounter> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<UCellCounter> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<UCellCounter> DeepCopy(this List<UCellCounter> source)
  {
    if (source == null)
      return null;

    var tmp1268 = new List<UCellCounter>(source.Count);
    foreach (var elem in source)
      tmp1268.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1268;
  }


  public static bool Equals(this List<UCellPlain> instance, object that)
  {
    if (!(that is List<UCellPlain> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<UCellPlain> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<UCellPlain> DeepCopy(this List<UCellPlain> source)
  {
    if (source == null)
      return null;

    var tmp1269 = new List<UCellPlain>(source.Count);
    foreach (var elem in source)
      tmp1269.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1269;
  }


  public static bool Equals(this List<UCellSerial> instance, object that)
  {
    if (!(that is List<UCellSerial> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<UCellSerial> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<UCellSerial> DeepCopy(this List<UCellSerial> source)
  {
    if (source == null)
      return null;

    var tmp1270 = new List<UCellSerial>(source.Count);
    foreach (var elem in source)
      tmp1270.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1270;
  }


  public static bool Equals(this List<byte[]> instance, object that)
  {
    if (!(that is List<byte[]> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<byte[]> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<byte[]> DeepCopy(this List<byte[]> source)
  {
    if (source == null)
      return null;

    var tmp1271 = new List<byte[]>(source.Count);
    foreach (var elem in source)
      tmp1271.Add((elem != null) ? elem.ToArray() : null);
    return tmp1271;
  }


  public static bool Equals(this List<kCells> instance, object that)
  {
    if (!(that is List<kCells> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<kCells> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<kCells> DeepCopy(this List<kCells> source)
  {
    if (source == null)
      return null;

    var tmp1272 = new List<kCells>(source.Count);
    foreach (var elem in source)
      tmp1272.Add((elem != null) ? elem.DeepCopy() : null);
    return tmp1272;
  }


  public static bool Equals(this List<long> instance, object that)
  {
    if (!(that is List<long> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<long> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<long> DeepCopy(this List<long> source)
  {
    if (source == null)
      return null;

    var tmp1273 = new List<long>(source.Count);
    foreach (var elem in source)
      tmp1273.Add(elem);
    return tmp1273;
  }


  public static bool Equals(this List<string> instance, object that)
  {
    if (!(that is List<string> other)) return false;
    if (ReferenceEquals(instance, other)) return true;

    return TCollections.Equals(instance, other);
  }


  public static int GetHashCode(this List<string> instance)
  {
    return TCollections.GetHashCode(instance);
  }


  public static List<string> DeepCopy(this List<string> source)
  {
    if (source == null)
      return null;

    var tmp1274 = new List<string>(source.Count);
    foreach (var elem in source)
      tmp1274.Add((elem != null) ? elem : null);
    return tmp1274;
  }


}