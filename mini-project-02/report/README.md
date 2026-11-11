# CPE333 Mini-Project 2 Report

This report presents the group’s Virtual Memory project: a Linux tool for inspecting virtual memory and counting process page faults. It explains the design, describes the test method, and discusses results from the merged implementation.

## Project details

- **Course:** CPE333 Operating Systems
- **Project:** Mini-Project 2 — Virtual Memory
- **Group:** มูลนิธิทนายตั้ม ช่วยสู้
- **Instructor:** Asst. Prof. Rajchawit Sarochawikasit
- **Members:** Seven students; names and student IDs appear on the cover.

## Source code

The kernel module and tools are maintained in the team repository at [merge commit 5fbcb767](https://github.com/beambeambeam/cpe333/tree/5fbcb7679298f36e2bae8b7cd3dbe6bf6a609800/mini-project-02). The follow-up regression run uses the updated `tests/regress.sh` in this PR, which checks for a new unload message in `dmesg` instead of relying on the last five kernel-log lines. This folder keeps the report and reviewed evidence alongside the project source without duplicating the implementation.

## Report files

The cover page lists student names and IDs, so `.gitignore` keeps it and the built editions out of this public repo. Share those files inside the group only.

| File | Purpose | In git |
|---|---|---|
| `CPE333_MiniProject2_Report.docx` | Editable Word or Google Docs version | No |
| `report.tex` | LaTeX source; reads the cover from `frontpage/frontpage.tex` | Yes |
| `frontpage/frontpage.tex` | Cover page with student names and IDs | No |
| `frontpage/frontpage.example.tex` | Cover template with placeholder members | Yes |
| `frontpage/assets/` | KMUTT logo for the cover | No |
| `output/pdf/CPE333_MiniProject2_Report.pdf` | Final A4 print and reading copy | No |
| `assets/` | Report diagrams | Yes |
| `.latexmkrc` | Makes `latexmk` use XeLaTeX, also from editors that pass `-pdf` | Yes |
| `evidence/post-merge-5fbcb76/` | Test logs, environment details, and eight Terminal captures | Yes |

## Layout and typography

The report follows the visual rhythm of PS04, PS06, and PS07: a centered KMUTT logo and title, generous spacing around the author and course details, bold black Problem/Step headings, green emphasis for key terms, and blue italic captions. Body text uses Sarabun 12 pt; Menlo is used for commands and code. The cover lists all seven group members and the instructor.

The print-ready PDF has 21 A4 portrait pages: cover on physical page 1, contents on page 2, report and references on physical pages 3–19 (numbered 1–17), and a two-page appendix on physical pages 20–21. It remains within the assignment's 25-page limit before the appendix. It uses the same Problem/Step hierarchy as the lab reports. Eight cropped Terminal screenshots appear beside the related discussion, including the demo, E7 timeline, and SIGCHLD cleanup runs. The `.txt` files keep searchable transcripts. The three `pfcount run` screenshots show the same PIDs and values as their transcripts. The appendix gives the full checkout, VM setup, build, live-process inspection, demo, regression, E1–E8 experiment, and cleanup sequence. The editable DOCX uses the same typography and portrait layout; Word or Google Docs may paginate it differently. Use the PDF for the intended print layout.

## Source and test results

- The report records tests on the merged `beambeambeam/cpe333` source at commit `5fbcb7679298f36e2bae8b7cd3dbe6bf6a609800`.
- The original measurements were collected in Lima with Ubuntu 24.04.5, Linux 6.8.0-142-generic, and ARM64. On 11 November 2026, we reran the full workflow from PR head `ed0a7a6` on Linux 6.8.0-147-generic: build/load, demo, all five regression checks, E1–E8, live-process comparison, a separate E7 timeline, and SIGCHLD cleanup all passed. The fresh logs are in `evidence/recheck-2026-11-11/`; the report’s measurement tables and chart remain tied to the original run.
- Rebuilt the LaTeX report on 11 November 2026. The output is 21 A4 pages; build details are in `evidence/recheck-2026-11-11/09-latex-build.txt`.
- Eight cropped Terminal screenshots show the required kernel options, module load, the demo, the `pfstat` report, regression checks, the E1–E8 run list, the E7 timeline, and SIGCHLD cleanup. Text files retain the recorded output, including commands, PIDs, elapsed times, exit statuses, and measurements where captured. The shell `pfstat` report (PID 1622) is kept only as a screenshot. The demo, regression, E7, and cleanup images match their transcripts by PID and result; the follow-up regression PID is 26873. The report explains why `faultgen`'s cumulative counts differ from `pfstat`'s baseline-adjusted counts.
- The original build log shows a BTF step but does not establish whether BTF data was generated. In the new verification log, BTF generation was skipped because the guest did not have `vmlinux`; the module still built and loaded. GCC reported different compiler executable names, but both versions were 13.3.0.
- The rerun ended by unloading the module, running `make clean`, and stopping Lima. The shutdown log records the instance stopped with exit status 0; Lima also emitted a closed-network-connection message as the host agent shut down.
- The recheck bundle includes cropped Terminal captures of the LaTeX build and matching regression run, alongside the searchable logs.
- GitHub Actions were not configured for the reviewed branch. Results in the report are from manual runs in Lima.
- The report describes the tested configuration and limitations, including the unmeasured probe overhead, descendant accounting scope, and the permission check that was not rerun after merge.

## Rebuild the PDF

Install XeLaTeX, `latexmk`, Sarabun, and Menlo. Copy `frontpage/frontpage.example.tex` to `frontpage/frontpage.tex`, fill in the members, and put `KMUTT_Logo.png` in `frontpage/assets/`; without `frontpage/frontpage.tex`, the PDF builds without a cover. From this folder, build into a temporary directory:

```sh
report_build_dir="$(mktemp -d -t cpe333-report)"
latexmk -xelatex -interaction=nonstopmode -halt-on-error \
  -outdir="$report_build_dir" report.tex
cp "$report_build_dir/report.pdf" output/pdf/CPE333_MiniProject2_Report.pdf
rm -rf "$report_build_dir"
```

The source uses `fontspec`, `polyglossia`, `geometry`, `graphicx`, `longtable`, `needspace`, `fancyhdr`, `titlesec`, `listings`, `xurl`, and `hyperref`. Shell escape is not required.

## Before submission

Prepare the separate 7–11 minute presentation and implementation demo required by the assignment.
