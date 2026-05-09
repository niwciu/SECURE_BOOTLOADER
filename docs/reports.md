# Reports

Reports are generated automatically by the [CI pipeline](https://github.com/niwciu/SECURE_BOOTLOADER/actions/workflows/CI_pipeline.yml) and published here on every merge to `main`.

---

## Code Coverage

Coverage is measured with **gcovr** across all unit-test suites.
Minimum threshold: **90 % line coverage**.

| Test suite | Report |
|---|---|
| `MAIN_APP` — bootloader core | [Open report](reports/MAIN_APP_coverage.html) |
| `CRC` — CRC-32 software implementation | [Open report](reports/CRC_coverage.html) |

---

## Code Complexity

Complexity is measured with **Lizard** on `src/`.  
Thresholds: CCN ≤ 12 · NLOC ≤ 30 · arguments ≤ 4.

[Open complexity report](reports/code_complexity_report.html)
