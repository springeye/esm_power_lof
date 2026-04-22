# Telegraphic Technical Documentation Style Guide

**Purpose**: Actionable writing constraints for concise, engineer-facing instruction files (AGENTS.md, CONTRIBUTING.md, etc.)

**Principle**: Brevity + Specificity. Every word earns its place. No filler, no hedging, no unnecessary context.

---

## CORE CONSTRAINTS

### 1. Line Length: 80 Characters Maximum
**Source**: [Google Markdown Style Guide](https://github.com/google/styleguide/blob/gh-pages/docguide/style.md#character-line-limit)

- **Why**: Matches code editor defaults; improves scannability; reduces cognitive load
- **Exceptions**: Links, tables, headings, code blocks (allowed to exceed)
- **Enforcement**: Wrap text before 80 chars; use reference links for long URLs

**Example (GOOD)**:
```markdown
- Use lazy numbering for long lists that may change
- Prefer fully numbered lists for small, stable lists
```

**Example (BAD)**:
```markdown
- When you have a list that is long and might change in the future, you should use lazy numbering instead of manually numbering each item
```

---

### 2. One Idea Per Sentence / Paragraph
**Source**: [GitHub Docs Best Practices](https://github.com/github/docs/blob/main/content/contributing/writing-for-github-docs/best-practices-for-github-docs.md#structure-content-for-readability)

- **Constraint**: Max 1 concept per sentence; max 1 topic per paragraph
- **Benefit**: Enables scanning; reduces re-reading; improves translation
- **Test**: Can you extract one clear takeaway from each sentence?

**Example (GOOD)**:
```markdown
## Commit Messages

Write descriptive commit messages.
Use past tense: "Fixed bug" not "Fix bug".
Start with a capital letter.
```

**Example (BAD)**:
```markdown
## Commit Messages

When writing commit messages, you should make sure they are descriptive, 
use past tense (like "Fixed bug" instead of "Fix bug"), and always start 
with a capital letter to maintain consistency across the project.
```

---

### 3. Section Density: Max 3-5 Items Per Section
**Source**: [Linux Kernel Coding Style](https://github.com/torvalds/linux/blob/master/Documentation/process/coding-style.rst) + [NASA Technical Documentation Style Guide](https://standards.nasa.gov/sites/default/files/standards/KSC/F/0/ksc-df-107_rev_f_07082017.pdf)

- **Constraint**: If a section exceeds 5 bullet points, split into subsections
- **Rationale**: Prevents "wall of text"; maintains focus; aids memory retention
- **Pattern**: Use H3 subheadings to break up dense lists

**Example (GOOD)**:
```markdown
### Commit Message Format
- Use past tense
- Start with capital letter
- Keep under 50 chars

### Commit Message Body
- Wrap at 72 characters
- Explain WHY, not WHAT
- Separate from subject with blank line
```

**Example (BAD)**:
```markdown
### Commit Messages
- Use past tense
- Start with capital letter
- Keep subject under 50 chars
- Wrap body at 72 characters
- Explain WHY, not WHAT
- Separate body from subject with blank line
- Use imperative mood
- Don't end with period
- Reference issues if applicable
```

---

### 4. Avoid Duplication Across Sections
**Source**: [Google Markdown Style Guide - Reference Links](https://github.com/google/styleguide/blob/gh-pages/docguide/style.md#use-reference-links-to-reduce-duplication)

- **Constraint**: If you repeat a concept 2+ times, extract to a shared section or reference
- **Pattern**: Use "See also" or "Related" sections; link instead of repeat
- **Benefit**: Single source of truth; easier maintenance; reduces cognitive load

**Example (GOOD)**:
```markdown
## Pull Request Process

1. Fork the repository
2. Create a feature branch
3. Follow [Commit Message Guidelines](#commit-message-guidelines)
4. Submit PR with description

## Commit Message Guidelines
[Detailed rules here]
```

**Example (BAD)**:
```markdown
## Pull Request Process

1. Fork the repository
2. Create a feature branch
3. Write commit messages in past tense, starting with capital letter, 
   keeping subject under 50 chars, wrapping body at 72 chars, explaining 
   WHY not WHAT
4. Submit PR with description

## Commit Message Guidelines

Write commit messages in past tense, starting with capital letter, 
keeping subject under 50 chars, wrapping body at 72 chars, explaining 
WHY not WHAT.
```

---

### 5. Active Voice + Imperative Mood
**Source**: [GitHub Docs Best Practices](https://github.com/github/docs/blob/main/content/contributing/writing-for-github-docs/best-practices-for-github-docs.md#write-for-readability)

- **Constraint**: Use commands, not descriptions
- **Pattern**: "Do X" not "You should do X" or "X should be done"
- **Benefit**: Shorter, clearer, more actionable

**Example (GOOD)**:
```markdown
- Fork the repository
- Create a feature branch
- Run tests before submitting
```

**Example (BAD)**:
```markdown
- You should fork the repository
- A feature branch should be created
- Tests should be run before submission
```

---

### 6. Eliminate Hedging Language
**Source**: [Ohio State Technical Communications](https://ohiostate.pressbooks.pub/feptechcomm/chapter/3-writing-style/)

- **Constraint**: Remove: "basically", "sort of", "kind of", "somewhat", "to a certain extent", "about"
- **Pattern**: State facts directly; if uncertain, say so explicitly
- **Benefit**: Increases authority; reduces ambiguity; saves words

**Example (GOOD)**:
```markdown
- Use 80-character line limits
- Wrap text at column 80
- Exceptions: links, tables, code blocks
```

**Example (BAD)**:
```markdown
- You should basically try to use 80-character line limits, sort of
- It's somewhat recommended to wrap text at about column 80
- There are kind of exceptions for things like links and tables
```

---

### 7. Parallel Structure in Lists
**Source**: [Microsoft Writing Style Guide](https://learn.microsoft.com/en-us/style-guide/welcome/)

- **Constraint**: All list items must start with same part of speech (verb, noun, etc.)
- **Pattern**: If first item is verb, all items are verbs; if noun, all nouns
- **Benefit**: Improves scannability; reduces cognitive friction

**Example (GOOD)**:
```markdown
- Clone the repository
- Create a feature branch
- Write tests
- Submit a pull request
```

**Example (BAD)**:
```markdown
- Clone the repository
- Creating a feature branch
- Write tests
- Submission of pull request
```

---

### 8. Front-Load Critical Information
**Source**: [AI Citation Behavior Research](https://dev.to/chudi_nnorukam/how-to-structure-content-so-ai-actually-cites-your-url-technical-guide-2531)

- **Constraint**: First 100 words must contain the core message
- **Pattern**: State conclusion first, then explain
- **Benefit**: Busy engineers scan; critical info must be visible immediately

**Example (GOOD)**:
```markdown
## Commit Messages

Write descriptive commit messages in past tense.

**Format**: `[TAG] Short summary (50 chars max)`

**Body** (optional):
- Wrap at 72 characters
- Explain WHY, not WHAT
```

**Example (BAD)**:
```markdown
## Commit Messages

When you're working on a project with multiple contributors, it's important 
to maintain consistency in how you communicate changes. One of the most 
important aspects of this is the commit message. A good commit message 
should be descriptive and written in past tense...
```

---

### 9. Use Formatting Elements to Break Density
**Source**: [GitHub Docs Best Practices](https://github.com/github/docs/blob/main/content/contributing/writing-for-github-docs/best-practices-for-github-docs.md#format-for-scannability)

- **Constraint**: No paragraph should exceed 3-4 sentences
- **Pattern**: Use bullet lists, tables, code blocks, bold text strategically
- **Benefit**: Reduces visual fatigue; improves scannability; aids retention

**Example (GOOD)**:
```markdown
## Setup

**Prerequisites**:
- Node.js 18+
- Git

**Steps**:
1. Clone the repo
2. Install dependencies: `npm install`
3. Run tests: `npm test`
```

**Example (BAD)**:
```markdown
## Setup

To get started, you'll need Node.js version 18 or higher and Git installed 
on your machine. Once you have those prerequisites, you should clone the 
repository, then install the dependencies using npm install, and finally 
run the tests using npm test to make sure everything is working correctly.
```

---

### 10. Avoid Unnecessary Context / Preamble
**Source**: [AI Citation Behavior Research](https://dev.to/chudi_nnorukam/how-to-structure-content-so-ai-actually-cites-your-url-technical-guide-2531)

- **Constraint**: Remove: "In this document...", "As mentioned above...", "It's important to note..."
- **Pattern**: Jump directly to actionable content
- **Benefit**: Saves 10-20% of word count; respects reader time

**Example (GOOD)**:
```markdown
## Reporting Issues

Search existing issues before creating a new one.
Include: version, OS, reproducible steps, expected vs. actual behavior.
```

**Example (BAD)**:
```markdown
## Reporting Issues

Before you proceed with reporting an issue, it's important to understand 
that we receive many bug reports. As mentioned in the guidelines above, 
you should search existing issues to avoid duplicates. When you do report 
an issue, please include the following information: your version, operating 
system, reproducible steps, and what you expected to see versus what you 
actually saw.
```

---

## PRACTICAL CHECKLIST FOR AGENTS.md

Apply these constraints when finalizing AGENTS.md:

- [ ] **Line length**: No line exceeds 80 chars (except links, code, tables)
- [ ] **One idea per sentence**: Each sentence contains exactly one concept
- [ ] **Section density**: No section has more than 5 bullet points (split if needed)
- [ ] **No duplication**: Repeated concepts are extracted to shared sections
- [ ] **Active voice**: All instructions use imperative mood ("Do X" not "You should X")
- [ ] **No hedging**: Removed "basically", "sort of", "somewhat", "about"
- [ ] **Parallel structure**: All list items start with same part of speech
- [ ] **Front-loaded info**: First 100 words contain core message
- [ ] **Formatting breaks**: No paragraph exceeds 3-4 sentences
- [ ] **No preamble**: Removed "In this document...", "As mentioned..."

---

## REAL-WORLD EXAMPLES

### Example 1: CONTRIBUTING.md (Telegraphic)

**Source**: [PyTorch Examples CONTRIBUTING.md](https://github.com/pytorch/examples/blob/main/CONTRIBUTING.md)

```markdown
# Contributing to examples

We want to make contributing as easy and transparent as possible.

## Pull Requests

We actively welcome your pull requests.

### For new examples

0. Create a GitHub issue proposing the example
1. Fork the repo and create your branch from `main`
2. If you've added code, add tests to `run_python_examples.sh`
3. Create a `README.md`
4. Add a card to `docs/source/index.rst`
5. Verify no doc build issues
6. Ensure tests pass locally
7. Complete the Contributor License Agreement

## Issues

Use GitHub issues to track public bugs.
Include: clear description, reproduction steps, expected vs. actual behavior.

## License

By contributing, you agree that your contributions are licensed under the 
same license as the project.
```

**Why this works**:
- ✅ Each bullet is one action
- ✅ No hedging language
- ✅ Imperative mood throughout
- ✅ Front-loaded critical info
- ✅ No unnecessary context

---

### Example 2: AGENTS.md (Telegraphic)

**Source**: Your current AGENTS.md (improved version)

```markdown
## Task Delegation Rules

### Principle: Prioritize delegation; simple tasks execute directly

- Analyze task type and complexity
- Complex/specialized tasks → delegate to best-fit agent
- Simple single-step operations → execute directly
- Decompose complex tasks into subtasks; delegate each

### Agent Responsibility Mapping

| Agent | Responsibility | Parameter |
|-------|---|---|
| `explore` | Code patterns, structure discovery | `subagent_type` |
| `librarian` | External docs, official APIs, OSS | `subagent_type` |
| `oracle` | Architecture decisions, hard debug | `subagent_type` |
| `metis` | Pre-analysis, requirements, intent | `subagent_type` |

### Parameter Rules (Zero Tolerance)

- Use `category` OR `subagent_type` (never both)
- Always pass `load_skills` (empty array if none)
- Expensive tasks: `run_in_background=true`
- UI tasks: MUST use `visual-engineering`

### Session Continuity

After each `task()` call, save the returned `session_id`.

- Task fails → use `session_id` to resume
- Same agent follow-up → always use `session_id`
- Never create duplicate sessions
```

**Why this works**:
- ✅ 80-char line limit maintained
- ✅ One concept per bullet
- ✅ Sections capped at 5 items
- ✅ No duplication (rules stated once)
- ✅ Imperative throughout
- ✅ Table breaks visual density
- ✅ No preamble

---

## MEASUREMENT: Before/After

### Before (Verbose)
```
Basically, when you're delegating tasks, you should try to prioritize 
delegation whenever possible, but for simple tasks that are kind of 
straightforward, you can execute them directly. It's important to analyze 
the task type and complexity to determine whether it should be delegated 
or not. If it's a complex task or a specialized task, you should delegate 
it to the agent that is best suited for that particular type of work.
```

**Metrics**:
- Word count: 72
- Sentences: 4
- Hedging words: 4 ("basically", "try", "kind of", "should")
- Clarity: Medium

### After (Telegraphic)
```
Analyze task type and complexity. Delegate complex/specialized tasks to 
best-fit agent. Execute simple single-step operations directly.
```

**Metrics**:
- Word count: 18 (75% reduction)
- Sentences: 3
- Hedging words: 0
- Clarity: High

---

## REFERENCES

1. **Google Markdown Style Guide** (80-char limit, reference links)
   - https://github.com/google/styleguide/blob/gh-pages/docguide/style.md

2. **GitHub Docs Best Practices** (one idea per sentence, active voice)
   - https://github.com/github/docs/blob/main/content/contributing/writing-for-github-docs/best-practices-for-github-docs.md

3. **Linux Kernel Coding Style** (section density, clarity over cleverness)
   - https://github.com/torvalds/linux/blob/master/Documentation/process/coding-style.rst

4. **NASA Technical Documentation Style Guide** (conciseness, organization)
   - https://standards.nasa.gov/sites/default/files/standards/KSC/F/0/ksc-df-107_rev_f_07082017.pdf

5. **Ohio State Technical Communications** (eliminate hedging, direct language)
   - https://ohiostate.pressbooks.pub/feptechcomm/chapter/3-writing-style/

6. **Microsoft Writing Style Guide** (parallel structure, active voice)
   - https://learn.microsoft.com/en-us/style-guide/welcome/

7. **AI Citation Behavior Research** (front-load info, avoid preamble)
   - https://dev.to/chudi_nnorukam/how-to-structure-content-so-ai-actually-cites-your-url-technical-guide-2531

---

## FINAL PRINCIPLE

**Telegraphic or die**: Every word must earn its place. If you can remove a word without losing meaning, remove it. If you can split a sentence into two, do it. If a section exceeds 5 items, break it up. Respect the reader's time.
