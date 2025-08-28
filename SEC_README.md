<h3>Snyk has created this PR to fix one or more vulnerable packages in the `pip` dependencies of this project.</h3>



#### Changes included in this PR

- Changes to the following files to upgrade the vulnerable dependencies to a fixed version:
    - rootfs/usr/local/lib/web/backend/requirements.txt


<details>
<summary>⚠️ <b>Warning</b></summary>

```
Werkzeug 2.2.3 has requirement MarkupSafe>=2.1.1, but you have MarkupSafe 1.1.1.
requests 2.22.0 has requirement urllib3!=1.25.0,!=1.25.1,<1.26,>=1.21.1, but you have urllib3 1.26.5.

```
</details>


#### Vulnerabilities that will be fixed





##### By pinning:
Severity                   | Priority Score (*)                   | Issue                   | Upgrade                   | Breaking Change                   | Exploit Maturity
:-------------------------:|-------------------------|:-------------------------|:-------------------------|:-------------------------|:-------------------------
![medium severity](https://res.cloudinary.com/snyk/image/upload/w_20,h_20/v1561977819/icon/m.png "medium severity")  |  **554/1000**  <br/> **Why?** Has a fix available, CVSS 6.8  | Insufficient Verification of Data Authenticity <br/>[SNYK-PYTHON-CERTIFI-3164749](https://snyk.io/vuln/SNYK-PYTHON-CERTIFI-3164749) |  `certifi:` <br> `2019.9.11 -> 2022.12.7` <br>  |  No  | No Known Exploit 
![high severity](https://res.cloudinary.com/snyk/image/upload/w_20,h_20/v1561977819/icon/h.png "high severity")  |  **599/1000**  <br/> **Why?** Has a fix available, CVSS 7.7  | HTTP Header Injection <br/>[SNYK-PYTHON-URLLIB3-1014645](https://snyk.io/vuln/SNYK-PYTHON-URLLIB3-1014645) |  `urllib3:` <br> `1.25.6 -> 1.26.5` <br>  |  No  | No Known Exploit 
![medium severity](https://res.cloudinary.com/snyk/image/upload/w_20,h_20/v1561977819/icon/m.png "medium severity")  |  **479/1000**  <br/> **Why?** Has a fix available, CVSS 5.3  | Regular Expression Denial of Service (ReDoS) <br/>[SNYK-PYTHON-URLLIB3-1533435](https://snyk.io/vuln/SNYK-PYTHON-URLLIB3-1533435) |  `urllib3:` <br> `1.25.6 -> 1.26.5` <br>  |  No  | No Known Exploit 
![medium severity](https://res.cloudinary.com/snyk/image/upload/w_20,h_20/v1561977819/icon/m.png "medium severity")  |  **509/1000**  <br/> **Why?** Has a fix available, CVSS 5.9  | Denial of Service (DoS) <br/>[SNYK-PYTHON-URLLIB3-559452](https://snyk.io/vuln/SNYK-PYTHON-URLLIB3-559452) |  `urllib3:` <br> `1.25.6 -> 1.26.5` <br>  |  No  | No Known Exploit 
![low severity](https://res.cloudinary.com/snyk/image/upload/w_20,h_20/v1561977819/icon/l.png "low severity")  |  **344/1000**  <br/> **Why?** Has a fix available, CVSS 2.6  | Access Restriction Bypass <br/>[SNYK-PYTHON-WERKZEUG-3319935](https://snyk.io/vuln/SNYK-PYTHON-WERKZEUG-3319935) |  `werkzeug:` <br> `0.16.0 -> 2.2.3` <br>  |  No  | No Known Exploit 
![high severity](https://res.cloudinary.com/snyk/image/upload/w_20,h_20/v1561977819/icon/h.png "high severity")  |  **589/1000**  <br/> **Why?** Has a fix available, CVSS 7.5  | Denial of Service (DoS) <br/>[SNYK-PYTHON-WERKZEUG-3319936](https://snyk.io/vuln/SNYK-PYTHON-WERKZEUG-3319936) |  `werkzeug:` <br> `0.16.0 -> 2.2.3` <br>  |  No  | No Known Exploit 

(*) Note that the real score may have changed since the PR was raised.




Some vulnerabilities couldn't be fully fixed and so Snyk will still find them when the project is tested again. This may be because the vulnerability existed within more than one direct dependency, but not all of the affected dependencies could be upgraded.


Check the changes in this PR to ensure they won't cause issues with your project.



------------



**Note:** *You are seeing this because you or someone else with access to this repository has authorized Snyk to open fix PRs.*

For more information:  <img src="https://api.segment.io/v1/pixel/track?data=eyJ3cml0ZUtleSI6InJyWmxZcEdHY2RyTHZsb0lYd0dUcVg4WkFRTnNCOUEwIiwiYW5vbnltb3VzSWQiOiI1MGU3OWFhMy1jNGFkLTQ2MjQtOTYzMi0yNDkyMDYyOTBkYzMiLCJldmVudCI6IlBSIHZpZXdlZCIsInByb3BlcnRpZXMiOnsicHJJZCI6IjUwZTc5YWEzLWM0YWQtNDYyNC05NjMyLTI0OTIwNjI5MGRjMyJ9fQ==" width="0" height="0"/>
🧐 [View latest project report](https://app.snyk.io/org/dannielshalev/project/2d42f184-efd7-4f3b-a34b-39b7fd0e450d?utm_source&#x3D;github&amp;utm_medium&#x3D;referral&amp;page&#x3D;fix-pr)

🛠 [Adjust project settings](https://app.snyk.io/org/dannielshalev/project/2d42f184-efd7-4f3b-a34b-39b7fd0e450d?utm_source&#x3D;github&amp;utm_medium&#x3D;referral&amp;page&#x3D;fix-pr/settings)

📚 [Read more about Snyk's upgrade and patch logic](https://support.snyk.io/hc/en-us/articles/360003891078-Snyk-patches-to-fix-vulnerabilities)

[//]: # (snyk:metadata:{"prId":"50e79aa3-c4ad-4624-9632-249206290dc3","prPublicId":"50e79aa3-c4ad-4624-9632-249206290dc3","dependencies":[{"name":"certifi","from":"2019.9.11","to":"2022.12.7"},{"name":"urllib3","from":"1.25.6","to":"1.26.5"},{"name":"werkzeug","from":"0.16.0","to":"2.2.3"}],"packageManager":"pip","projectPublicId":"2d42f184-efd7-4f3b-a34b-39b7fd0e450d","projectUrl":"https://app.snyk.io/org/dannielshalev/project/2d42f184-efd7-4f3b-a34b-39b7fd0e450d?utm_source=github&utm_medium=referral&page=fix-pr","type":"user-initiated","patch":[],"vulns":["SNYK-PYTHON-CERTIFI-3164749","SNYK-PYTHON-URLLIB3-1014645","SNYK-PYTHON-URLLIB3-1533435","SNYK-PYTHON-URLLIB3-559452","SNYK-PYTHON-WERKZEUG-3319935","SNYK-PYTHON-WERKZEUG-3319936"],"upgrade":[],"isBreakingChange":false,"env":"prod","prType":"fix","templateVariants":["pr-warning-shown","priorityScore"],"priorityScoreList":[554,599,479,509,344,589],"remediationStrategy":"vuln"})

---

**Learn how to fix vulnerabilities with free interactive lessons:**

 🦉 [Regular Expression Denial of Service (ReDoS)](https://learn.snyk.io/lessons/redos/javascript/?loc&#x3D;fix-pr)
 🦉 [Access Restriction Bypass](https://learn.snyk.io/lessons/broken-access-control/python/?loc&#x3D;fix-pr)
 🦉 [Denial of Service (DoS)](https://learn.snyk.io/lessons/no-rate-limiting/python/?loc&#x3D;fix-pr)


 <h3>Snyk has created this PR to fix one or more vulnerable packages in the `yarn` dependencies of this project.</h3>



#### Changes included in this PR

- Changes to the following files to upgrade the vulnerable dependencies to a fixed version:
    - web/package.json
    - web/yarn.lock


#### Note for [zero-installs](https://yarnpkg.com/features/zero-installs) users

If you are using the Yarn feature [zero-installs](https://yarnpkg.com/features/zero-installs) that was introduced in Yarn V2, note that this PR does not update the `.yarn/cache/` directory meaning this code cannot be pulled and immediately developed on as one would expect for a zero-install project - you will need to run `yarn` to update the contents of the `./yarn/cache` directory. 
If you are not using zero-install you can ignore this as your flow should likely be unchanged.


#### Vulnerabilities that will be fixed
##### With an upgrade:
Severity                   | Priority Score (*)                   | Issue                   | Breaking Change                   | Exploit Maturity
:-------------------------:|-------------------------|:-------------------------|:-------------------------|:-------------------------
![high severity](https://res.cloudinary.com/snyk/image/upload/w_20,h_20/v1561977819/icon/h.png "high severity")  |  **696/1000**  <br/> **Why?** Proof of Concept exploit, Has a fix available, CVSS 7.5  | Regular Expression Denial of Service (ReDoS) <br/>[SNYK-JS-AXIOS-1579269](https://snyk.io/vuln/SNYK-JS-AXIOS-1579269) |  No  | Proof of Concept 

(*) Note that the real score may have changed since the PR was raised.











Check the changes in this PR to ensure they won't cause issues with your project.



------------



**Note:** *You are seeing this because you or someone else with access to this repository has authorized Snyk to open fix PRs.*

For more information:  <img src="https://api.segment.io/v1/pixel/track?data=eyJ3cml0ZUtleSI6InJyWmxZcEdHY2RyTHZsb0lYd0dUcVg4WkFRTnNCOUEwIiwiYW5vbnltb3VzSWQiOiI1ZmZiZTNiZS1kMzAzLTRhYjktYWNkMi1iODczYjAxZjdjZGIiLCJldmVudCI6IlBSIHZpZXdlZCIsInByb3BlcnRpZXMiOnsicHJJZCI6IjVmZmJlM2JlLWQzMDMtNGFiOS1hY2QyLWI4NzNiMDFmN2NkYiJ9fQ==" width="0" height="0"/>
🧐 [View latest project report](https://app.snyk.io/org/dannielshalev/project/1f02abc0-324a-411b-ba42-2a338069ae7d?utm_source&#x3D;github&amp;utm_medium&#x3D;referral&amp;page&#x3D;fix-pr)

🛠 [Adjust project settings](https://app.snyk.io/org/dannielshalev/project/1f02abc0-324a-411b-ba42-2a338069ae7d?utm_source&#x3D;github&amp;utm_medium&#x3D;referral&amp;page&#x3D;fix-pr/settings)

📚 [Read more about Snyk's upgrade and patch logic](https://support.snyk.io/hc/en-us/articles/360003891078-Snyk-patches-to-fix-vulnerabilities)

[//]: # (snyk:metadata:{"prId":"5ffbe3be-d303-4ab9-acd2-b873b01f7cdb","prPublicId":"5ffbe3be-d303-4ab9-acd2-b873b01f7cdb","dependencies":[{"name":"axios","from":"0.21.1","to":"0.21.3"}],"packageManager":"yarn","projectPublicId":"1f02abc0-324a-411b-ba42-2a338069ae7d","projectUrl":"https://app.snyk.io/org/dannielshalev/project/1f02abc0-324a-411b-ba42-2a338069ae7d?utm_source=github&utm_medium=referral&page=fix-pr","type":"user-initiated","patch":[],"vulns":["SNYK-JS-AXIOS-1579269"],"upgrade":["SNYK-JS-AXIOS-1579269"],"isBreakingChange":false,"env":"prod","prType":"fix","templateVariants":["updated-fix-title","priorityScore"],"priorityScoreList":[696],"remediationStrategy":"vuln"})

---

**Learn how to fix vulnerabilities with free interactive lessons:**

 🦉 [Regular Expression Denial of Service (ReDoS)](https://learn.snyk.io/lessons/redos/javascript/?loc&#x3D;fix-pr)
