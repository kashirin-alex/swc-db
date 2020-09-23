/**
 * Autogenerated by Thrift Compiler (0.13.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */

/// <summary>
/// Column Key Sequences
/// </summary>
public enum KeySeq
{
  /// <summary>
  /// Unknown/Unrecognized Type
  /// </summary>
  UNKNOWN = 0,
  /// <summary>
  /// The Lexical Key Order Sequence
  /// </summary>
  LEXIC = 1,
  /// <summary>
  /// The Volumetric Key Order Sequence
  /// </summary>
  VOLUME = 2,
  /// <summary>
  /// The by Fractions Count on Lexical Key Order Sequence
  /// </summary>
  FC_LEXIC = 3,
  /// <summary>
  /// The by Fractions Count on Volumetric Key Order Sequence
  /// </summary>
  FC_VOLUME = 4,
}