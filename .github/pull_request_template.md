#
**Summary**
###

"What changed and why"



#
**Test plan**
###

- [ ] Built locally (describe configure/make if relevant)
- [ ] Ran local tests (`make test` and/or other — note what)
- [ ] Included `[TEST COMMIT]` in a commit message to trigger GitHub Actions CI (optional; CI is skipped without it)
- [ ] N/A — docs-only / no binary change



#
**Checklist**
###

- [ ] I understand and agree to [CONTRIBUTING.md](https://github.com/kashirin-alex/swc-db/blob/master/CONTRIBUTING.md)
- [ ] New source files include the project copyright header
- [ ] Thrift behavior changes edit IDL and/or hand-written wrappers — not `thriftgen-*` / `gen-*`
- [ ] Code changes follow Layer A/B expectations in [evals/STANDARDS_CLARITY_EVALUATION.md](https://github.com/kashirin-alex/swc-db/blob/master/evals/STANDARDS_CLARITY_EVALUATION.md) §9 when applicable
- [ ] Docs changes under `docs/` keep frontmatter (`title`, `sort`) and prefer integrity fixes (versions, links) over drive-by rewrites
- [ ] Docs PRs that touch releases / CMake / SQL / Thrift Comp / `src/etc/swcdb` defaults update matching pages (see CONTRIBUTING.md docs gates)
