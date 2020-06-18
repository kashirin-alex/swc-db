
<h1>Thrift module: Service</h1>
<table class="table-bordered table-striped table-condensed"><thead><tr><th>Module</th><th>Services</th><th>Data types</th><th>Constants</th></tr></thead><tbody>
<tr>
<td>Service</td><td><a href="#Svc_Service">Service</a><br/>
<ul>
<li><a href="#Fn_Service_compact_columns">compact_columns</a></li>
<li><a href="#Fn_Service_list_columns">list_columns</a></li>
<li><a href="#Fn_Service_mng_column">mng_column</a></li>
<li><a href="#Fn_Service_scan">scan</a></li>
<li><a href="#Fn_Service_scan_rslt_on">scan_rslt_on</a></li>
<li><a href="#Fn_Service_scan_rslt_on_column">scan_rslt_on_column</a></li>
<li><a href="#Fn_Service_scan_rslt_on_fraction">scan_rslt_on_fraction</a></li>
<li><a href="#Fn_Service_scan_rslt_on_key">scan_rslt_on_key</a></li>
<li><a href="#Fn_Service_sql_compact_columns">sql_compact_columns</a></li>
<li><a href="#Fn_Service_sql_list_columns">sql_list_columns</a></li>
<li><a href="#Fn_Service_sql_mng_column">sql_mng_column</a></li>
<li><a href="#Fn_Service_sql_query">sql_query</a></li>
<li><a href="#Fn_Service_sql_select">sql_select</a></li>
<li><a href="#Fn_Service_sql_select_rslt_on_column">sql_select_rslt_on_column</a></li>
<li><a href="#Fn_Service_sql_select_rslt_on_fraction">sql_select_rslt_on_fraction</a></li>
<li><a href="#Fn_Service_sql_select_rslt_on_key">sql_select_rslt_on_key</a></li>
<li><a href="#Fn_Service_sql_update">sql_update</a></li>
<li><a href="#Fn_Service_update">update</a></li>
<li><a href="#Fn_Service_updater_close">updater_close</a></li>
<li><a href="#Fn_Service_updater_create">updater_create</a></li>
</ul>
</td>
<td><a href="#Struct_CCell">CCell</a><br/>
<a href="#Typedef_CCells">CCells</a><br/>
<a href="#Struct_Cell">Cell</a><br/>
<a href="#Typedef_Cells">Cells</a><br/>
<a href="#Struct_CellsGroup">CellsGroup</a><br/>
<a href="#Enum_CellsResult">CellsResult</a><br/>
<a href="#Typedef_ColCells">ColCells</a><br/>
<a href="#Enum_ColumnMng">ColumnMng</a><br/>
<a href="#Enum_ColumnType">ColumnType</a><br/>
<a href="#Enum_Comp">Comp</a><br/>
<a href="#Struct_CompactResult">CompactResult</a><br/>
<a href="#Typedef_CompactResults">CompactResults</a><br/>
<a href="#Enum_EncodingType">EncodingType</a><br/>
<a href="#Struct_Exception">Exception</a><br/>
<a href="#Struct_FCell">FCell</a><br/>
<a href="#Struct_FCells">FCells</a><br/>
<a href="#Enum_Flag">Flag</a><br/>
<a href="#Struct_KCell">KCell</a><br/>
<a href="#Typedef_KCells">KCells</a><br/>
<a href="#Typedef_Key">Key</a><br/>
<a href="#Struct_Schema">Schema</a><br/>
<a href="#Enum_SchemaFunc">SchemaFunc</a><br/>
<a href="#Typedef_Schemas">Schemas</a><br/>
<a href="#Struct_SpecColumn">SpecColumn</a><br/>
<a href="#Struct_SpecFlags">SpecFlags</a><br/>
<a href="#Enum_SpecFlagsOpt">SpecFlagsOpt</a><br/>
<a href="#Struct_SpecFraction">SpecFraction</a><br/>
<a href="#Struct_SpecInterval">SpecInterval</a><br/>
<a href="#Typedef_SpecKey">SpecKey</a><br/>
<a href="#Struct_SpecScan">SpecScan</a><br/>
<a href="#Struct_SpecSchemas">SpecSchemas</a><br/>
<a href="#Struct_SpecTimestamp">SpecTimestamp</a><br/>
<a href="#Struct_SpecValue">SpecValue</a><br/>
<a href="#Typedef_UCCells">UCCells</a><br/>
<a href="#Struct_UCell">UCell</a><br/>
<a href="#Typedef_UCells">UCells</a><br/>
<a href="#Struct_kCells">kCells</a><br/>
</td>
<td></td>
</tr></tbody></table>
<hr/><h2 id="Enumerations">Enumerations</h2>
<div class="definition"><h3 id="Enum_ColumnMng">Enumeration: ColumnMng</h3>
<br/><table class="table-bordered table-striped table-condensed">
<tr><td><code>CREATE</code></td><td><code>3</code></td><td>
</td></tr>
<tr><td><code>DELETE</code></td><td><code>5</code></td><td>
</td></tr>
<tr><td><code>MODIFY</code></td><td><code>7</code></td><td>
</td></tr>
</table></div>
<div class="definition"><h3 id="Enum_ColumnType">Enumeration: ColumnType</h3>
<br/><table class="table-bordered table-striped table-condensed">
<tr><td><code>UNKNOWN</code></td><td><code>0</code></td><td>
</td></tr>
<tr><td><code>PLAIN</code></td><td><code>1</code></td><td>
</td></tr>
<tr><td><code>COUNTER_I64</code></td><td><code>2</code></td><td>
</td></tr>
<tr><td><code>COUNTER_I32</code></td><td><code>3</code></td><td>
</td></tr>
<tr><td><code>COUNTER_I16</code></td><td><code>4</code></td><td>
</td></tr>
<tr><td><code>COUNTER_I8</code></td><td><code>5</code></td><td>
</td></tr>
<tr><td><code>CELL_DEFINED</code></td><td><code>15</code></td><td>
</td></tr>
</table></div>
<div class="definition"><h3 id="Enum_EncodingType">Enumeration: EncodingType</h3>
<br/><table class="table-bordered table-striped table-condensed">
<tr><td><code>DEFAULT</code></td><td><code>0</code></td><td>
</td></tr>
<tr><td><code>PLAIN</code></td><td><code>1</code></td><td>
</td></tr>
<tr><td><code>ZLIB</code></td><td><code>2</code></td><td>
</td></tr>
<tr><td><code>SNAPPY</code></td><td><code>3</code></td><td>
</td></tr>
</table></div>
<div class="definition"><h3 id="Enum_SchemaFunc">Enumeration: SchemaFunc</h3>
<br/><table class="table-bordered table-striped table-condensed">
<tr><td><code>CREATE</code></td><td><code>3</code></td><td>
</td></tr>
<tr><td><code>DELETE</code></td><td><code>5</code></td><td>
</td></tr>
<tr><td><code>MODIFY</code></td><td><code>7</code></td><td>
</td></tr>
</table></div>
<div class="definition"><h3 id="Enum_Comp">Enumeration: Comp</h3>
<br/><table class="table-bordered table-striped table-condensed">
<tr><td><code>NONE</code></td><td><code>0</code></td><td>
</td></tr>
<tr><td><code>PF</code></td><td><code>1</code></td><td>
</td></tr>
<tr><td><code>GT</code></td><td><code>2</code></td><td>
</td></tr>
<tr><td><code>GE</code></td><td><code>3</code></td><td>
</td></tr>
<tr><td><code>EQ</code></td><td><code>4</code></td><td>
</td></tr>
<tr><td><code>LE</code></td><td><code>5</code></td><td>
</td></tr>
<tr><td><code>LT</code></td><td><code>6</code></td><td>
</td></tr>
<tr><td><code>NE</code></td><td><code>7</code></td><td>
</td></tr>
<tr><td><code>RE</code></td><td><code>8</code></td><td>
</td></tr>
<tr><td><code>VGT</code></td><td><code>9</code></td><td>
</td></tr>
<tr><td><code>VGE</code></td><td><code>10</code></td><td>
</td></tr>
<tr><td><code>VLE</code></td><td><code>11</code></td><td>
</td></tr>
<tr><td><code>VLT</code></td><td><code>12</code></td><td>
</td></tr>
</table></div>
<div class="definition"><h3 id="Enum_SpecFlagsOpt">Enumeration: SpecFlagsOpt</h3>
<br/><table class="table-bordered table-striped table-condensed">
<tr><td><code>NONE</code></td><td><code>0</code></td><td>
</td></tr>
<tr><td><code>LIMIT_BY_KEYS</code></td><td><code>1</code></td><td>
</td></tr>
<tr><td><code>OFFSET_BY_KEYS</code></td><td><code>4</code></td><td>
</td></tr>
<tr><td><code>ONLY_KEYS</code></td><td><code>8</code></td><td>
</td></tr>
<tr><td><code>ONLY_DELETES</code></td><td><code>10</code></td><td>
</td></tr>
</table></div>
<div class="definition"><h3 id="Enum_Flag">Enumeration: Flag</h3>
<br/><table class="table-bordered table-striped table-condensed">
<tr><td><code>NONE</code></td><td><code>0</code></td><td>
</td></tr>
<tr><td><code>INSERT</code></td><td><code>1</code></td><td>
</td></tr>
<tr><td><code>DELETE</code></td><td><code>2</code></td><td>
</td></tr>
<tr><td><code>DELETE_VERSION</code></td><td><code>3</code></td><td>
</td></tr>
</table></div>
<div class="definition"><h3 id="Enum_CellsResult">Enumeration: CellsResult</h3>
<br/><table class="table-bordered table-striped table-condensed">
<tr><td><code>IN_LIST</code></td><td><code>0</code></td><td>
</td></tr>
<tr><td><code>ON_COLUMN</code></td><td><code>1</code></td><td>
</td></tr>
<tr><td><code>ON_KEY</code></td><td><code>2</code></td><td>
</td></tr>
<tr><td><code>ON_FRACTION</code></td><td><code>3</code></td><td>
</td></tr>
</table></div>
<hr/><h2 id="Typedefs">Type declarations</h2>
<div class="definition"><h3 id="Typedef_Schemas">Typedef: Schemas</h3>
<p><strong>Base type:</strong>&nbsp;<code>list&lt;<code><a href="#Struct_Schema">Schema</a></code>&gt;</code></p>
</div>
<div class="definition"><h3 id="Typedef_Key">Typedef: Key</h3>
<p><strong>Base type:</strong>&nbsp;<code>list&lt;<code>binary</code>&gt;</code></p>
</div>
<div class="definition"><h3 id="Typedef_SpecKey">Typedef: SpecKey</h3>
<p><strong>Base type:</strong>&nbsp;<code>list&lt;<code><a href="#Struct_SpecFraction">SpecFraction</a></code>&gt;</code></p>
</div>
<div class="definition"><h3 id="Typedef_UCells">Typedef: UCells</h3>
<p><strong>Base type:</strong>&nbsp;<code>list&lt;<code><a href="#Struct_UCell">UCell</a></code>&gt;</code></p>
</div>
<div class="definition"><h3 id="Typedef_UCCells">Typedef: UCCells</h3>
<p><strong>Base type:</strong>&nbsp;<code>map&lt;<code>i64</code>, <code><a href="#Struct_UCells">UCells</a></code>&gt;</code></p>
</div>
<div class="definition"><h3 id="Typedef_Cells">Typedef: Cells</h3>
<p><strong>Base type:</strong>&nbsp;<code>list&lt;<code><a href="#Struct_Cell">Cell</a></code>&gt;</code></p>
</div>
<div class="definition"><h3 id="Typedef_ColCells">Typedef: ColCells</h3>
<p><strong>Base type:</strong>&nbsp;<code>list&lt;<code><a href="#Struct_CCell">CCell</a></code>&gt;</code></p>
</div>
<div class="definition"><h3 id="Typedef_CCells">Typedef: CCells</h3>
<p><strong>Base type:</strong>&nbsp;<code>map&lt;<code>string</code>, <code><a href="#Struct_ColCells">ColCells</a></code>&gt;</code></p>
</div>
<div class="definition"><h3 id="Typedef_KCells">Typedef: KCells</h3>
<p><strong>Base type:</strong>&nbsp;<code>list&lt;<code><a href="#Struct_kCells">kCells</a></code>&gt;</code></p>
</div>
<div class="definition"><h3 id="Typedef_CompactResults">Typedef: CompactResults</h3>
<p><strong>Base type:</strong>&nbsp;<code>list&lt;<code><a href="#Struct_CompactResult">CompactResult</a></code>&gt;</code></p>
</div>
<hr/><h2 id="Structs">Data structures</h2>
<div class="definition"><h3 id="Struct_Exception">Exception: Exception</h3>
<table class="table-bordered table-striped table-condensed"><thead><tr><th>Key</th><th>Field</th><th>Type</th><th>Description</th><th>Requiredness</th><th>Default value</th></tr></thead><tbody>
<tr><td>1</td><td>code</td><td><code>i32</code></td><td></td><td>default</td><td></td></tr>
<tr><td>2</td><td>message</td><td><code>string</code></td><td></td><td>default</td><td></td></tr>
</tbody></table><br/></div><div class="definition"><h3 id="Struct_Schema">Struct: Schema</h3>
<table class="table-bordered table-striped table-condensed"><thead><tr><th>Key</th><th>Field</th><th>Type</th><th>Description</th><th>Requiredness</th><th>Default value</th></tr></thead><tbody>
<tr><td>1</td><td>cid</td><td><code>i64</code></td><td></td><td>optional</td><td></td></tr>
<tr><td>2</td><td>col_name</td><td><code>string</code></td><td></td><td>optional</td><td></td></tr>
<tr><td>3</td><td>col_type</td><td><code><a href="#Enum_ColumnType">ColumnType</a></code></td><td></td><td>optional</td><td></td></tr>
<tr><td>4</td><td>cell_versions</td><td><code>i32</code></td><td></td><td>optional</td><td></td></tr>
<tr><td>5</td><td>cell_ttl</td><td><code>i32</code></td><td></td><td>optional</td><td></td></tr>
<tr><td>6</td><td>blk_encoding</td><td><code><a href="#Enum_EncodingType">EncodingType</a></code></td><td></td><td>optional</td><td></td></tr>
<tr><td>7</td><td>blk_size</td><td><code>i32</code></td><td></td><td>optional</td><td></td></tr>
<tr><td>8</td><td>blk_cells</td><td><code>i32</code></td><td></td><td>optional</td><td></td></tr>
<tr><td>9</td><td>cs_replication</td><td><code>i8</code></td><td></td><td>optional</td><td></td></tr>
<tr><td>10</td><td>cs_size</td><td><code>i32</code></td><td></td><td>optional</td><td></td></tr>
<tr><td>11</td><td>cs_max</td><td><code>i8</code></td><td></td><td>optional</td><td></td></tr>
<tr><td>12</td><td>log_rollout_ratio</td><td><code>i8</code></td><td></td><td>optional</td><td></td></tr>
<tr><td>13</td><td>compact_percent</td><td><code>i8</code></td><td></td><td>optional</td><td></td></tr>
<tr><td>14</td><td>revision</td><td><code>i64</code></td><td></td><td>optional</td><td></td></tr>
</tbody></table><br/></div><div class="definition"><h3 id="Struct_SpecSchemas">Struct: SpecSchemas</h3>
<table class="table-bordered table-striped table-condensed"><thead><tr><th>Key</th><th>Field</th><th>Type</th><th>Description</th><th>Requiredness</th><th>Default value</th></tr></thead><tbody>
<tr><td>1</td><td>cids</td><td><code>list&lt;<code>i64</code>&gt;</code></td><td></td><td>optional</td><td></td></tr>
<tr><td>2</td><td>names</td><td><code>list&lt;<code>string</code>&gt;</code></td><td></td><td>optional</td><td></td></tr>
</tbody></table><br/></div><div class="definition"><h3 id="Struct_SpecFlags">Struct: SpecFlags</h3>
<table class="table-bordered table-striped table-condensed"><thead><tr><th>Key</th><th>Field</th><th>Type</th><th>Description</th><th>Requiredness</th><th>Default value</th></tr></thead><tbody>
<tr><td>1</td><td>limit</td><td><code>i64</code></td><td></td><td>optional</td><td></td></tr>
<tr><td>2</td><td>offset</td><td><code>i64</code></td><td></td><td>optional</td><td></td></tr>
<tr><td>3</td><td>max_versions</td><td><code>i32</code></td><td></td><td>optional</td><td></td></tr>
<tr><td>4</td><td>max_buffer</td><td><code>i32</code></td><td></td><td>optional</td><td></td></tr>
<tr><td>5</td><td>options</td><td><code>i8</code></td><td></td><td>optional</td><td></td></tr>
</tbody></table><br/></div><div class="definition"><h3 id="Struct_SpecFraction">Struct: SpecFraction</h3>
<table class="table-bordered table-striped table-condensed"><thead><tr><th>Key</th><th>Field</th><th>Type</th><th>Description</th><th>Requiredness</th><th>Default value</th></tr></thead><tbody>
<tr><td>1</td><td>comp</td><td><code><a href="#Enum_Comp">Comp</a></code></td><td></td><td>default</td><td></td></tr>
<tr><td>2</td><td>f</td><td><code>binary</code></td><td></td><td>default</td><td></td></tr>
</tbody></table><br/></div><div class="definition"><h3 id="Struct_SpecValue">Struct: SpecValue</h3>
<table class="table-bordered table-striped table-condensed"><thead><tr><th>Key</th><th>Field</th><th>Type</th><th>Description</th><th>Requiredness</th><th>Default value</th></tr></thead><tbody>
<tr><td>1</td><td>comp</td><td><code><a href="#Enum_Comp">Comp</a></code></td><td></td><td>default</td><td></td></tr>
<tr><td>2</td><td>v</td><td><code>binary</code></td><td></td><td>default</td><td></td></tr>
</tbody></table><br/></div><div class="definition"><h3 id="Struct_SpecTimestamp">Struct: SpecTimestamp</h3>
<table class="table-bordered table-striped table-condensed"><thead><tr><th>Key</th><th>Field</th><th>Type</th><th>Description</th><th>Requiredness</th><th>Default value</th></tr></thead><tbody>
<tr><td>1</td><td>comp</td><td><code><a href="#Enum_Comp">Comp</a></code></td><td></td><td>default</td><td></td></tr>
<tr><td>2</td><td>ts</td><td><code>i64</code></td><td></td><td>default</td><td></td></tr>
</tbody></table><br/></div><div class="definition"><h3 id="Struct_SpecInterval">Struct: SpecInterval</h3>
<table class="table-bordered table-striped table-condensed"><thead><tr><th>Key</th><th>Field</th><th>Type</th><th>Description</th><th>Requiredness</th><th>Default value</th></tr></thead><tbody>
<tr><td>1</td><td>range_begin</td><td><code><a href="#Struct_Key">Key</a></code></td><td></td><td>optional</td><td></td></tr>
<tr><td>2</td><td>range_end</td><td><code><a href="#Struct_Key">Key</a></code></td><td></td><td>optional</td><td></td></tr>
<tr><td>3</td><td>range_offset</td><td><code><a href="#Struct_Key">Key</a></code></td><td></td><td>optional</td><td></td></tr>
<tr><td>4</td><td>offset_key</td><td><code><a href="#Struct_Key">Key</a></code></td><td></td><td>optional</td><td></td></tr>
<tr><td>5</td><td>offset_rev</td><td><code>i64</code></td><td></td><td>optional</td><td></td></tr>
<tr><td>6</td><td>key_start</td><td><code><a href="#Struct_SpecKey">SpecKey</a></code></td><td></td><td>optional</td><td></td></tr>
<tr><td>7</td><td>key_finish</td><td><code><a href="#Struct_SpecKey">SpecKey</a></code></td><td></td><td>optional</td><td></td></tr>
<tr><td>8</td><td>value</td><td><code><a href="#Struct_SpecValue">SpecValue</a></code></td><td></td><td>optional</td><td></td></tr>
<tr><td>9</td><td>ts_start</td><td><code><a href="#Struct_SpecTimestamp">SpecTimestamp</a></code></td><td></td><td>optional</td><td></td></tr>
<tr><td>10</td><td>ts_finish</td><td><code><a href="#Struct_SpecTimestamp">SpecTimestamp</a></code></td><td></td><td>optional</td><td></td></tr>
<tr><td>11</td><td>flags</td><td><code><a href="#Struct_SpecFlags">SpecFlags</a></code></td><td></td><td>optional</td><td></td></tr>
</tbody></table><br/></div><div class="definition"><h3 id="Struct_SpecColumn">Struct: SpecColumn</h3>
<table class="table-bordered table-striped table-condensed"><thead><tr><th>Key</th><th>Field</th><th>Type</th><th>Description</th><th>Requiredness</th><th>Default value</th></tr></thead><tbody>
<tr><td>1</td><td>cid</td><td><code>i64</code></td><td></td><td>default</td><td></td></tr>
<tr><td>2</td><td>intervals</td><td><code>list&lt;<code><a href="#Struct_SpecInterval">SpecInterval</a></code>&gt;</code></td><td></td><td>default</td><td></td></tr>
</tbody></table><br/></div><div class="definition"><h3 id="Struct_SpecScan">Struct: SpecScan</h3>
<table class="table-bordered table-striped table-condensed"><thead><tr><th>Key</th><th>Field</th><th>Type</th><th>Description</th><th>Requiredness</th><th>Default value</th></tr></thead><tbody>
<tr><td>1</td><td>columns</td><td><code>list&lt;<code><a href="#Struct_SpecColumn">SpecColumn</a></code>&gt;</code></td><td></td><td>default</td><td></td></tr>
<tr><td>2</td><td>flags</td><td><code><a href="#Struct_SpecFlags">SpecFlags</a></code></td><td></td><td>optional</td><td></td></tr>
</tbody></table><br/></div><div class="definition"><h3 id="Struct_UCell">Struct: UCell</h3>
<table class="table-bordered table-striped table-condensed"><thead><tr><th>Key</th><th>Field</th><th>Type</th><th>Description</th><th>Requiredness</th><th>Default value</th></tr></thead><tbody>
<tr><td>1</td><td>f</td><td><code><a href="#Enum_Flag">Flag</a></code></td><td></td><td>default</td><td></td></tr>
<tr><td>2</td><td>k</td><td><code><a href="#Struct_Key">Key</a></code></td><td></td><td>default</td><td></td></tr>
<tr><td>3</td><td>ts</td><td><code>i64</code></td><td></td><td>optional</td><td></td></tr>
<tr><td>4</td><td>ts_desc</td><td><code>bool</code></td><td></td><td>optional</td><td></td></tr>
<tr><td>5</td><td>v</td><td><code>binary</code></td><td></td><td>optional</td><td></td></tr>
</tbody></table><br/></div><div class="definition"><h3 id="Struct_Cell">Struct: Cell</h3>
<table class="table-bordered table-striped table-condensed"><thead><tr><th>Key</th><th>Field</th><th>Type</th><th>Description</th><th>Requiredness</th><th>Default value</th></tr></thead><tbody>
<tr><td>1</td><td>c</td><td><code>string</code></td><td></td><td>default</td><td></td></tr>
<tr><td>2</td><td>k</td><td><code><a href="#Struct_Key">Key</a></code></td><td></td><td>default</td><td></td></tr>
<tr><td>3</td><td>ts</td><td><code>i64</code></td><td></td><td>default</td><td></td></tr>
<tr><td>4</td><td>v</td><td><code>binary</code></td><td></td><td>optional</td><td></td></tr>
</tbody></table><br/></div><div class="definition"><h3 id="Struct_CCell">Struct: CCell</h3>
<table class="table-bordered table-striped table-condensed"><thead><tr><th>Key</th><th>Field</th><th>Type</th><th>Description</th><th>Requiredness</th><th>Default value</th></tr></thead><tbody>
<tr><td>1</td><td>k</td><td><code><a href="#Struct_Key">Key</a></code></td><td></td><td>default</td><td></td></tr>
<tr><td>2</td><td>ts</td><td><code>i64</code></td><td></td><td>default</td><td></td></tr>
<tr><td>3</td><td>v</td><td><code>binary</code></td><td></td><td>optional</td><td></td></tr>
</tbody></table><br/></div><div class="definition"><h3 id="Struct_KCell">Struct: KCell</h3>
<table class="table-bordered table-striped table-condensed"><thead><tr><th>Key</th><th>Field</th><th>Type</th><th>Description</th><th>Requiredness</th><th>Default value</th></tr></thead><tbody>
<tr><td>1</td><td>c</td><td><code>string</code></td><td></td><td>default</td><td></td></tr>
<tr><td>2</td><td>ts</td><td><code>i64</code></td><td></td><td>default</td><td></td></tr>
<tr><td>3</td><td>v</td><td><code>binary</code></td><td></td><td>optional</td><td></td></tr>
</tbody></table><br/></div><div class="definition"><h3 id="Struct_kCells">Struct: kCells</h3>
<table class="table-bordered table-striped table-condensed"><thead><tr><th>Key</th><th>Field</th><th>Type</th><th>Description</th><th>Requiredness</th><th>Default value</th></tr></thead><tbody>
<tr><td>1</td><td>k</td><td><code><a href="#Struct_Key">Key</a></code></td><td></td><td>default</td><td></td></tr>
<tr><td>2</td><td>cells</td><td><code>list&lt;<code><a href="#Struct_KCell">KCell</a></code>&gt;</code></td><td></td><td>default</td><td></td></tr>
</tbody></table><br/></div><div class="definition"><h3 id="Struct_FCell">Struct: FCell</h3>
<table class="table-bordered table-striped table-condensed"><thead><tr><th>Key</th><th>Field</th><th>Type</th><th>Description</th><th>Requiredness</th><th>Default value</th></tr></thead><tbody>
<tr><td>1</td><td>c</td><td><code>string</code></td><td></td><td>default</td><td></td></tr>
<tr><td>2</td><td>ts</td><td><code>i64</code></td><td></td><td>default</td><td></td></tr>
<tr><td>3</td><td>v</td><td><code>binary</code></td><td></td><td>optional</td><td></td></tr>
</tbody></table><br/></div><div class="definition"><h3 id="Struct_FCells">Struct: FCells</h3>
<table class="table-bordered table-striped table-condensed"><thead><tr><th>Key</th><th>Field</th><th>Type</th><th>Description</th><th>Requiredness</th><th>Default value</th></tr></thead><tbody>
<tr><td>1</td><td>f</td><td><code>map&lt;<code>binary</code>, <code><a href="#Struct_FCells">FCells</a></code>&gt;</code></td><td></td><td>default</td><td></td></tr>
<tr><td>2</td><td>cells</td><td><code>list&lt;<code><a href="#Struct_FCell">FCell</a></code>&gt;</code></td><td></td><td>optional</td><td></td></tr>
</tbody></table><br/></div><div class="definition"><h3 id="Struct_CellsGroup">Struct: CellsGroup</h3>
<table class="table-bordered table-striped table-condensed"><thead><tr><th>Key</th><th>Field</th><th>Type</th><th>Description</th><th>Requiredness</th><th>Default value</th></tr></thead><tbody>
<tr><td>1</td><td>cells</td><td><code><a href="#Struct_Cells">Cells</a></code></td><td></td><td>optional</td><td></td></tr>
<tr><td>2</td><td>ccells</td><td><code><a href="#Struct_CCells">CCells</a></code></td><td></td><td>optional</td><td></td></tr>
<tr><td>3</td><td>kcells</td><td><code><a href="#Struct_KCells">KCells</a></code></td><td></td><td>optional</td><td></td></tr>
<tr><td>4</td><td>fcells</td><td><code><a href="#Struct_FCells">FCells</a></code></td><td></td><td>optional</td><td></td></tr>
</tbody></table><br/></div><div class="definition"><h3 id="Struct_CompactResult">Struct: CompactResult</h3>
<table class="table-bordered table-striped table-condensed"><thead><tr><th>Key</th><th>Field</th><th>Type</th><th>Description</th><th>Requiredness</th><th>Default value</th></tr></thead><tbody>
<tr><td>1</td><td>cid</td><td><code>i64</code></td><td></td><td>default</td><td></td></tr>
<tr><td>2</td><td>err</td><td><code>i32</code></td><td></td><td>default</td><td></td></tr>
</tbody></table><br/></div><hr/><h2 id="Services">Services</h2>
<h3 id="Svc_Service">Service: Service</h3>
<div class="definition"><h4 id="Fn_Service_sql_mng_column">Function: Service.sql_mng_column</h4>
<pre><code>void</code> sql_mng_column(<code>string</code> sql)
    throws <code><a href="#Struct_Exception">Exception</a></code>
</pre></div><div class="definition"><h4 id="Fn_Service_sql_list_columns">Function: Service.sql_list_columns</h4>
<pre><code><a href="#Struct_Schemas">Schemas</a></code> sql_list_columns(<code>string</code> sql)
    throws <code><a href="#Struct_Exception">Exception</a></code>
</pre></div><div class="definition"><h4 id="Fn_Service_sql_compact_columns">Function: Service.sql_compact_columns</h4>
<pre><code><a href="#Struct_CompactResults">CompactResults</a></code> sql_compact_columns(<code>string</code> sql)
    throws <code><a href="#Struct_Exception">Exception</a></code>
</pre></div><div class="definition"><h4 id="Fn_Service_sql_select">Function: Service.sql_select</h4>
<pre><code><a href="#Struct_Cells">Cells</a></code> sql_select(<code>string</code> sql)
    throws <code><a href="#Struct_Exception">Exception</a></code>
</pre></div><div class="definition"><h4 id="Fn_Service_sql_select_rslt_on_column">Function: Service.sql_select_rslt_on_column</h4>
<pre><code><a href="#Struct_CCells">CCells</a></code> sql_select_rslt_on_column(<code>string</code> sql)
    throws <code><a href="#Struct_Exception">Exception</a></code>
</pre></div><div class="definition"><h4 id="Fn_Service_sql_select_rslt_on_key">Function: Service.sql_select_rslt_on_key</h4>
<pre><code><a href="#Struct_KCells">KCells</a></code> sql_select_rslt_on_key(<code>string</code> sql)
    throws <code><a href="#Struct_Exception">Exception</a></code>
</pre></div><div class="definition"><h4 id="Fn_Service_sql_select_rslt_on_fraction">Function: Service.sql_select_rslt_on_fraction</h4>
<pre><code><a href="#Struct_FCells">FCells</a></code> sql_select_rslt_on_fraction(<code>string</code> sql)
    throws <code><a href="#Struct_Exception">Exception</a></code>
</pre></div><div class="definition"><h4 id="Fn_Service_sql_query">Function: Service.sql_query</h4>
<pre><code><a href="#Struct_CellsGroup">CellsGroup</a></code> sql_query(<code>string</code> sql,
                     <code><a href="#Enum_CellsResult">CellsResult</a></code> rslt)
    throws <code><a href="#Struct_Exception">Exception</a></code>
</pre></div><div class="definition"><h4 id="Fn_Service_sql_update">Function: Service.sql_update</h4>
<pre><code>void</code> sql_update(<code>string</code> sql,
                <code>i64</code> updater_id = 0)
    throws <code><a href="#Struct_Exception">Exception</a></code>
</pre></div><div class="definition"><h4 id="Fn_Service_updater_create">Function: Service.updater_create</h4>
<pre><code>i64</code> updater_create(<code>i32</code> buffer_size)
    throws <code><a href="#Struct_Exception">Exception</a></code>
</pre></div><div class="definition"><h4 id="Fn_Service_updater_close">Function: Service.updater_close</h4>
<pre><code>void</code> updater_close(<code>i64</code> id)
    throws <code><a href="#Struct_Exception">Exception</a></code>
</pre></div><div class="definition"><h4 id="Fn_Service_update">Function: Service.update</h4>
<pre><code>void</code> update(<code><a href="#Struct_UCCells">UCCells</a></code> cells,
            <code>i64</code> updater_id = 0)
    throws <code><a href="#Struct_Exception">Exception</a></code>
</pre></div><div class="definition"><h4 id="Fn_Service_mng_column">Function: Service.mng_column</h4>
<pre><code>void</code> mng_column(<code><a href="#Enum_SchemaFunc">SchemaFunc</a></code> func,
                <code><a href="#Struct_Schema">Schema</a></code> schema)
    throws <code><a href="#Struct_Exception">Exception</a></code>
</pre></div><div class="definition"><h4 id="Fn_Service_list_columns">Function: Service.list_columns</h4>
<pre><code><a href="#Struct_Schemas">Schemas</a></code> list_columns(<code><a href="#Struct_SpecSchemas">SpecSchemas</a></code> spec)
    throws <code><a href="#Struct_Exception">Exception</a></code>
</pre></div><div class="definition"><h4 id="Fn_Service_compact_columns">Function: Service.compact_columns</h4>
<pre><code><a href="#Struct_CompactResults">CompactResults</a></code> compact_columns(<code><a href="#Struct_SpecSchemas">SpecSchemas</a></code> spec)
    throws <code><a href="#Struct_Exception">Exception</a></code>
</pre></div><div class="definition"><h4 id="Fn_Service_scan">Function: Service.scan</h4>
<pre><code><a href="#Struct_Cells">Cells</a></code> scan(<code><a href="#Struct_SpecScan">SpecScan</a></code> spec)
    throws <code><a href="#Struct_Exception">Exception</a></code>
</pre></div><div class="definition"><h4 id="Fn_Service_scan_rslt_on_column">Function: Service.scan_rslt_on_column</h4>
<pre><code><a href="#Struct_CCells">CCells</a></code> scan_rslt_on_column(<code><a href="#Struct_SpecScan">SpecScan</a></code> spec)
    throws <code><a href="#Struct_Exception">Exception</a></code>
</pre></div><div class="definition"><h4 id="Fn_Service_scan_rslt_on_key">Function: Service.scan_rslt_on_key</h4>
<pre><code><a href="#Struct_KCells">KCells</a></code> scan_rslt_on_key(<code><a href="#Struct_SpecScan">SpecScan</a></code> spec)
    throws <code><a href="#Struct_Exception">Exception</a></code>
</pre></div><div class="definition"><h4 id="Fn_Service_scan_rslt_on_fraction">Function: Service.scan_rslt_on_fraction</h4>
<pre><code><a href="#Struct_FCells">FCells</a></code> scan_rslt_on_fraction(<code><a href="#Struct_SpecScan">SpecScan</a></code> spec)
    throws <code><a href="#Struct_Exception">Exception</a></code>
</pre></div><div class="definition"><h4 id="Fn_Service_scan_rslt_on">Function: Service.scan_rslt_on</h4>
<pre><code><a href="#Struct_CellsGroup">CellsGroup</a></code> scan_rslt_on(<code><a href="#Struct_SpecScan">SpecScan</a></code> spec,
                        <code><a href="#Enum_CellsResult">CellsResult</a></code> rslt)
    throws <code><a href="#Struct_Exception">Exception</a></code>
</pre></div>
