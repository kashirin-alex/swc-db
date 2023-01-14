/**
 * Autogenerated by Thrift Compiler (0.17.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
package org.swcdb.thrift.gen;


/**
 * The available logical Comparators, plus extended logic options applied with 'v' for VOLUME
 */
public enum Comp implements org.apache.thrift.TEnum {
  /**
   * [         ]  :   none               (no comparison applied)
   */
  NONE(0),
  /**
   * [  =^     ]  :   -pf [prefix]       (starts-with)
   */
  PF(1),
  /**
   * [ &gt;    ]  :   -gt                (greater-than)
   */
  GT(2),
  /**
   * [ &gt;=   ]  :   -ge                (greater-equal)
   */
  GE(3),
  /**
   * [  =      ]  :   -eq                (equal)
   */
  EQ(4),
  /**
   * [ &lt;=   ]  :   -le                (lower-equal)
   */
  LE(5),
  /**
   * [ &lt;    ]  :   -lt                (lower-than)
   */
  LT(6),
  /**
   * [  !=     ]  :   -ne                (not-equal)
   */
  NE(7),
  /**
   * [  re     ]  :   -re [r,regexp]     (regular-expression)
   */
  RE(8),
  /**
   * [ v&gt;   ]  :   -vgt               (vol greater-than)
   */
  VGT(9),
  /**
   * [ v&gt;=  ]  :   -vge               (vol greater-equal)
   */
  VGE(10),
  /**
   * [ v&lt;=  ]  :   -vle               (vol lower-equal)
   */
  VLE(11),
  /**
   * [ v&lt;   ]  :   -vlt               (vol lower-than)
   */
  VLT(12),
  /**
   * [ %&gt;   ]  :   -subset [sbs]      (subset)
   */
  SBS(13),
  /**
   * [ &lt;%   ]  :   -supset [sps]      (superset)
   */
  SPS(14),
  /**
   * [ ~&gt;   ]  :   -posubset [posbs]  (eq/part ordered subset)
   */
  POSBS(15),
  /**
   * [ &lt;~   ]  :   -posupset [posps]  (eq/part ordered superset)
   */
  POSPS(16),
  /**
   * [ -&gt;   ]  :   -fosubset [fosbs]  (eq/full ordered subset)
   */
  FOSBS(17),
  /**
   * [ &lt;-   ]  :   -fosupset [fosps]  (eq/full ordered superset)
   */
  FOSPS(18),
  /**
   * [ :&lt;   ]  :   -fip  (fraction include prior)
   */
  FIP(19),
  /**
   * [ :       ]  :   -fi   (fraction include)
   */
  FI(20);

  private final int value;

  private Comp(int value) {
    this.value = value;
  }

  /**
   * Get the integer value of this enum value, as defined in the Thrift IDL.
   */
  @Override
  public int getValue() {
    return value;
  }

  /**
   * Find a the enum type by its integer value, as defined in the Thrift IDL.
   * @return null if the value is not found.
   */
  @org.apache.thrift.annotation.Nullable
  public static Comp findByValue(int value) { 
    switch (value) {
      case 0:
        return NONE;
      case 1:
        return PF;
      case 2:
        return GT;
      case 3:
        return GE;
      case 4:
        return EQ;
      case 5:
        return LE;
      case 6:
        return LT;
      case 7:
        return NE;
      case 8:
        return RE;
      case 9:
        return VGT;
      case 10:
        return VGE;
      case 11:
        return VLE;
      case 12:
        return VLT;
      case 13:
        return SBS;
      case 14:
        return SPS;
      case 15:
        return POSBS;
      case 16:
        return POSPS;
      case 17:
        return FOSBS;
      case 18:
        return FOSPS;
      case 19:
        return FIP;
      case 20:
        return FI;
      default:
        return null;
    }
  }
}