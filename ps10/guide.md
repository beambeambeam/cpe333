# PS10 guide – Dining Philosophers vs. Cigarette Smokers

PS10 has no program and no experiment. The deliverable is a presentation PDF of 4–7 pages that explains the Cigarette Smokers problem, compares it with the Dining Philosophers problem, and cites reliable sources. This guide gives the page plan, the source list, and the build and check steps.

## 1. Prepare the workspace

The requirement slide is in `materials/requirements/PS10_2026.pdf`. The presentation is the group's own work, so it lives in this directory:

```text
presentation.tex   # Beamer source, 16:9
presentation.pdf   # the file to submit
```

Git tracks the PDF and ignores the LaTeX auxiliary files. There is no `results/` directory because there are no screenshots or outputs to collect.

## 2. Page budget

The cover page and the reference page do not count toward the 4–7 pages. The deck has 8 pages in total: cover, 6 content slides, references.

| Page | Slide | Requirement it covers |
|---|---|---|
| 1 | Cover | – |
| 2 | The Cigarette Smokers problem | Problem of Cigarette Smokers |
| 3 | Why it is hard: the natural solution deadlocks | Problem of Cigarette Smokers |
| 4 | The solution: helper threads called pushers | Problem of Cigarette Smokers |
| 5 | Recap: the Dining Philosophers problem | Difference |
| 6 | Difference: Cigarette Smokers vs. Dining Philosophers | Difference |
| 7 | What the differences mean | Difference |
| 8 | References | – |

Keep the content slides between 4 and 7. If you add a slide, count it.

## 3. Sources

Cite on the slide with `\cite{key}` and list every source on the reference page. Prefer primary sources (papers, textbooks). Use Wikipedia and blogs only to find primary sources. Do not cite them.

| Key | Source | Used for |
|---|---|---|
| `patil` | S. Patil, Computation Structures Group Memo 57, MIT Project MAC, 1971 | Origin of the Cigarette Smokers problem |
| `parnas` | D. L. Parnas, *Communications of the ACM* 18(3), 1975, pp. 181–183 | Response to Patil, solution without conditionals |
| `downey` | A. B. Downey, *The Little Book of Semaphores*, 2nd ed., v2.2.2, sections 4.5 and 4.6 | Problem statement, restrictions, natural solution, pushers, Dining Philosophers fixes |
| `dijkstra` | E. W. Dijkstra, *Acta Informatica* 1(2), 1971, pp. 115–138 | Published form of Dining Philosophers |
| `tanenbaum` | A. S. Tanenbaum and H. Bos, *Modern Operating Systems*, 4th ed., section 2.5.1 | Dining Philosophers and its standard solution |
| `coffman` | E. G. Coffman, M. J. Elphick, A. Shoshani, *ACM Computing Surveys* 3(2), 1971, pp. 67–78 | Four conditions for deadlock |

Check before you submit:

- The text of Downey's sections 4.5 and 4.6 was read in full. The text of the Patil memo and the Parnas paper was not read. The deck states their claims as Downey reports them. Read both if you can.
- The Patil memo is a scanned PDF. The URL on the reference page comes from a course archive and may stop working.
- Page numbers for Tanenbaum differ between editions. The deck cites the section only. Add page numbers from your own copy if you want them.

## 4. Build and check

```bash
cd ps10
pdflatex presentation.tex
pdflatex presentation.tex   # second run resolves the citations
pdfinfo presentation.pdf | grep Pages
```

Expect `Pages: 8`. Then render the pages and look at each one:

```bash
pdftoppm -r 80 -png presentation.pdf /tmp/ps10
```

Check that:

- no citation shows as `[?]`;
- no text or diagram runs into the footer or off the page;
- every claim in the comparison table has a citation or follows from a cited source.

## 5. Submit

Submit `presentation.pdf`. The group must have 3–4 people. The names and IDs on the cover page are set in `\author{...}` in `presentation.tex`. Change them if the group is different.
