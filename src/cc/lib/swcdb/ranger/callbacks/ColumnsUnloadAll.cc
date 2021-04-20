/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


namespace SWC { namespace Ranger { namespace Callback {


ColumnsUnloadAll::ColumnsUnloadAll(bool validation)
                                  : ColumnsUnload(nullptr, nullptr, false),
                                    validation(validation)  {
}

void ColumnsUnloadAll::unloaded(RangePtr range) {
  if(validation)
    SWC_LOGF(LOG_WARN,
              "Unload-Validation Range(cid=%lu rid=%lu) remained",
              range->cfg->cid, range->rid);
}

void ColumnsUnloadAll::unloaded(const ColumnPtr& col) {
  if(validation)
    SWC_LOGF(LOG_WARN,
      "Unload-Validation Column(cid=%lu ranges=%lu use-count=%ld) remained",
      col->cfg->cid, col->ranges_count(), col->cfg.use_count());
  ColumnsUnload::unloaded(col);
}

}}}
