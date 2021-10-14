/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


namespace SWC { namespace Ranger { namespace Callback {


SWC_CAN_INLINE
ColumnsUnloadAll::ColumnsUnloadAll(bool a_validation)
      : ColumnsUnload(nullptr, nullptr, false, 0, 0),
        validation(a_validation)  {
}

void ColumnsUnloadAll::unloaded(RangePtr range) {
  if(validation)
    SWC_LOGF(LOG_WARN,
      "Unload-Validation Range(cid=" SWC_FMT_LU " rid=" SWC_FMT_LU
      ") remained",
      range->cfg->cid, range->rid);
}

void ColumnsUnloadAll::unloaded(const ColumnPtr& col) {
  if(validation)
    SWC_LOGF(LOG_WARN,
      "Unload-Validation Column(cid=" SWC_FMT_LU " ranges=" SWC_FMT_LU
      " use-count=" SWC_FMT_LU ") remained",
      col->cfg->cid, col->ranges_count(), size_t(col->cfg.use_count()));
  ColumnsUnload::unloaded(col);
}

}}}
