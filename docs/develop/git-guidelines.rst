Git Guidelines
==============

Git really shines when it is used to its strengths. It is not a glorified backup
system; rather, the commit graph should tell a story. Ideally, the Git log is
all that should be necessary to catch up with the most recent developments.
Making the history more valuable for the long-term requires that we put some
more effort into it while we are working on it however.

1. Git Common Sense: https://www.git-tower.com/learn/git/ebook/en/command-line/appendix/best-practices
2. Working as a Team (Branching + Rebasing): https://web.archive.org/web/20180822170112/http://kentnguyen.com/development/visualized-git-practices-for-team/
3. Making Sense of Git (Fundamentals): http://eagain.net/articles/git-for-computer-scientists/
4. Best Practices on Commits: https://chris.beams.io/posts/git-commit/
5. Further Best Practices: https://sethrobertson.github.io/GitBestPractices/

The above five articles are highly recommended to read, starting from the
basics and ending with best practices. It is from these articles that many of
the following guidelines are derived.

1. Commit related changes [1]_

   - Commit logical, self-contained units
   - Commit often
   - Don't commit half-done work
   - Don't commit commented out code
   - Test before you commit

2. Write good commit messages [1]_ [4]_

   - Separate subject from body with a blank line
   - Limit the subject line to 50 characters
   - Capitalize the subject line
   - Do not end the subject line with a period
   - Use the imperative mood in the subject line
   - Wrap the body in 72 characters
   - Use the body to explain what and why vs. how

3. Rebase when you can [2]_ [5]_

   - Keep the Git history as clean of merge commits as possible
   - Do not rewrite history that has been pushed upstream

4. Understand the principles of how Git works [3]_

   - Git is effectively a directed acyclic graph
   - Understanding this will help you understand the rest of Git

5. Learn the Git CLI [2]_ [4]_ [5]_

   - Consider reading the `Pro Git <https://git-scm.com/book>`__ book

Project Guidelines
~~~~~~~~~~~~~~~~~~

If you want to contribute to Cloe development, you may maintain a separate fork
of Cloe or you may create a branch within. If you want to maintain your
development in a branch in our repository, then please name your branch
``<responsible>/<topic>``.
This helps us when trying to identify which branches to prune.

Several examples:

- ``feature/random-changes`` -- bad, "feature" tells us nothing new, and neither
  does "random-changes". Sometimes, the Git Service Provider has special support
  for prefixes, which may provide an exception.
- ``jsmith/foobar-controller`` -- good, we know who maintains this branch and
  what the branch is about.
- ``feature/ISSUE-31337`` -- bad, "feature" tells us nothing new, and we don't
  want the indirection that "ISSUE-31337" forces on us.
- ``teamx/ISSUE-7944-add-timestamps-to-radar-objs`` -- good, "teamx" tells us
  the team responsible, and while "ISSUE-7944" provides the link, the text
  following it tells us the gist of that link.
- ``trash/psa-demo`` -- good, if we don't know whether it's useful anymore,
  then "trash" tells us we can probably delete it.

If a branch is going to live for a longer time, or never be merged back to
master, please discuss this with us first. Everyone who clones a Git project
clones all branches by default. We might all be better off if you fork the
project instead, this is effectively identical from the workflow, but better
for everyone not interested in your branch.

.. [1] https://www.git-tower.com/learn/git/ebook/en/command-line/appendix/best-practices
.. [2] https://kentnguyen.com/development/visualized-git-practices-for-team/
.. [3] http://eagain.net/articles/git-for-computer-scientists/
.. [4] https://chris.beams.io/posts/git-commit/
.. [5] https://sethrobertson.github.io/GitBestPractices/
