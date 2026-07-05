
# Take-Home Tech Evaluation: Market Data Replay and Order Book Reconstruction

## 1. Overview

You are given a simulated exchange market data dataset captured in [PCAP](https://www.ietf.org/archive/id/draft-gharris-opsawg-pcap-01.html) format, along with a protocol specification and a starter C++ codebase.

The goal of this challenge is to improve the provided implementation so that it can robustly process the input data, reconstruct order book snapshots, and produce the required CSV output.

This challenge is designed to evaluate engineering fundamentals in a realistic data-systems setting, including parsing, state management, debugging, testing, robustness, and performance reasoning.

You may use any tools you find helpful, including AI tools / LLMs. However, you must complete the challenge yourself. Direct help from other people, including having someone else design, implement, or debug substantial parts of the solution for you, is not allowed.

## 2. What We Provide

You will receive the following:

- `README.md` — this document
- `Knowledge.md` — primer on order books and market data concepts
- `Specification.md` — protocol and data format specification
- `cpp/` — starter C++ codebase
- `Makefile` — build and run convenience targets for the C++ solution
- `input.pcap` — a market data capture of approximately 300MB
- `ground_truth.csv` — the correct output of this task. Your code should produce output that matches this ground truth as closely as possible.

## 3. Your Task

Starting from the provided C++ codebase, build a more correct, robust, and maintainable solution that:

1. Reads and parses the PCAP input
2. Extracts and decodes market data messages from UDP packets
3. Maintains the relevant order book state
4. Produces reconstructed snapshot output in the required CSV format
5. Handles imperfect or inconsistent input as reasonably as possible

In addition, include a write-up covering:

- Any assumptions you made for ambiguous cases
- What you discovered about the data, including any notable characteristics or interesting patterns
- Your thought process without simply restating the final solution or implementation details
- How you interpreted the challenge and what you took away from it
- Performance benchmark report (if any)

> [!NOTE]
> The write-up will carry significant weight in the evaluation, so please prepare it carefully.
>
> If you use an LLM to draft or polish the write-up:
> - Make sure it accurately reflects your own thinking and intended message.
> - Make sure it is clear, coherent, and written for a human reader.

This is **not** a greenfield implementation challenge. We expect you to work from the provided code and improve it substantially.

### Expected Output

Your program should output a CSV file containing reconstructed book snapshots in the required schema described in `Specification.md`.

Please follow the output schema exactly.

### Requirements

- The core implementation must be in **C++**.
- The code uses C++20 and should compile with GCC 13 or newer.
- Build on top of the provided starter code in `cpp/`.
- If you are not already comfortable with C++, learn what you need during the challenge. The level of C++ required here is not especially advanced, and with effective LLM use, this should not be a major obstacle.

### Tools

- We strongly encourage the use of AI tools / LLMs throughout the challenge.
- We encourage candidates to use modern development workflows, including iterative AI-assisted development where useful.
- You may use auxiliary tools for debugging, testing, analysis, and development.
- If you use an LLM in a material way, please briefly describe how you used it in your write-up. 

## 4. What We Care About

We are evaluating not only whether your code "works", but also how you approach a messy real-world engineering problem.

### Performance Expectations

There is **no strict performance target** for this challenge.

However, we do expect you to:

- identify major performance bottlenecks
- make reasonable improvements where appropriate
- include a short performance improvement discussion in your report

We care more about whether you can identify and reason about performance issues than whether you aggressively optimize everything.


### Assumptions and Ambiguity

In real systems, specifications and data are often imperfect.

If you encounter ambiguous cases or need to make assumptions, please:

- make a reasonable engineering decision
- document it clearly in your report

We value thoughtful, explicit assumptions much more than silent guesswork.

### Testing and Hidden Evaluation

Your submission will be evaluated on datasets beyond the provided input.

This means:

- overfitting to a single visible dataset is unlikely to work well
- robustness matters
- hard-coded behavior or narrow patching will be viewed negatively

### Evaluation Criteria

We will evaluate submissions along the following dimensions:

- correctness of reconstruction
- robustness under edge cases and imperfect input
- code structure and maintainability
- quality of tests and validation
- **clarity and depth of the report**
- quality of performance analysis and improvement work

## 5. Deliverables

Please submit the following:

- [ ] **A write-up**
- [ ] All source code required to build and run your solution
- [ ] Any tests you wrote, along with brief instructions if needed
- [ ] (Optional) Conversation with the LLM

You do **not** need to submit the input data files or output CSV files.

Before submitting, verify:

- [ ] Code compiles with `make build`
- [ ] Code runs end-to-end with `make run` (reads `input.pcap`, writes `output.csv`)
